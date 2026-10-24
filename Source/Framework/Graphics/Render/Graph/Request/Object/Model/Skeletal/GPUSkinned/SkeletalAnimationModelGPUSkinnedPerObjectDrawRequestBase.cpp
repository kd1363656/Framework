#include "SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase.h"

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::BeginFrame()
{
    // 参照が途切れているstd::weak_ptrを削除する
    m_drawRequestDataSmartPointerVectorList.RemoveExpiredElements();

    // 新しいフレームの準備(インスタンスの表を送り直す、Viewのカリング状態を戻す)
    m_gpuDrivenDrawer.BeginFrame();

    m_currentDrawViewIndex = k_defaultDrawViewIndex;
}

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource&)
{
    // DispatchGPUCullingでGPUが作った描画コマンドを、1回の命令でまとめて実行する
    m_gpuDrivenDrawer.ExecuteIndirect(a_renderer, a_rootSignature, m_currentDrawViewIndex);
}

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::AddDrawRequest(const std::shared_ptr<Struct::SkeletalAnimationModelGPUSkinnedDrawRequestData>& a_drawRequestData)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

    m_drawRequestDataSmartPointerVectorList.Add(a_drawRequestData);
}

bool FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView)
{
    // 1フレームに1回だけ、描画申請からインスタンスやMeshの表を作ってGPUへ送る
    if (m_gpuDrivenDrawer.IsTableUploadRequired())
    {
        const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

        m_gpuDrivenTableBuilder.Clear                        ();
        m_gpuDrivenTableBuilder.SetTargetMaterialStaticTypeID(GetVALTargetMaterialStaticTypeID());

        for (const auto& l_drawRequestDataElement : m_drawRequestDataSmartPointerVectorList.GetREFElementDataList())
        {
            const auto& l_drawRequestData = l_drawRequestDataElement.m_type.lock();

            if (!l_drawRequestData) { continue; }

            const auto& l_skeletalAnimationModelRecord = l_drawRequestData->m_skeletalAnimationModelRecord.lock();

            if (!l_skeletalAnimationModelRecord) { continue; }

            // モデルのベイク済みアニメーションを取得し、まだGPUへ送っていなければコピー命令を積む
            const auto& l_bakedSkinning = FindOrCreateBakedSkinning(*l_skeletalAnimationModelRecord);

            if (!l_bakedSkinning) { continue; }

            l_bakedSkinning->UploadIfNeeded(l_directCommandList);

            // インスタンスの共通部分に加えて、再生時刻から「何コマ目か」を設定する
            auto l_instance = m_gpuDrivenTableBuilder.CreateInstance(l_drawRequestData->m_worldMatrix, l_drawRequestData->m_worldInverseTransposeMatrix);

            l_instance.m_animationFrameIndex = l_bakedSkinning->FetchVALFrameIndex(l_drawRequestData->m_motionIndex, l_drawRequestData->m_animationTimeSecond);

            m_gpuDrivenTableBuilder.AddInstance(*l_skeletalAnimationModelRecord, l_instance, &l_bakedSkinning->GetREFSkinningInfo());
        }

        m_gpuDrivenDrawer.UploadFrameTable(a_renderer, m_gpuDrivenTableBuilder.Build());
    }

    return m_gpuDrivenDrawer.DispatchCulling(a_renderer, a_viewIndex, a_cullingView);
}

std::shared_ptr<FWK::Graphics::SkeletalAnimationBakedSkinning> FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::FindOrCreateBakedSkinning(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const
{
    // 既にベイク済みなら、それを使う(同じモデルを使う全ての描画で共有する)
    if (const auto& l_bakedSkinning = a_skeletalAnimationModelRecord.GetREFBakedSkinning();
        l_bakedSkinning)
    {
        return l_bakedSkinning;
    }

    auto l_bakedSkinning = std::make_shared<SkeletalAnimationBakedSkinning>();

    FWK_ASSERT_RETURN_VALUE_IF(!l_bakedSkinning->Create(a_skeletalAnimationModelRecord), "アニメーションのベイクに失敗しました。", nullptr);

    a_skeletalAnimationModelRecord.SetBakedSkinning(l_bakedSkinning);

    return l_bakedSkinning;
}