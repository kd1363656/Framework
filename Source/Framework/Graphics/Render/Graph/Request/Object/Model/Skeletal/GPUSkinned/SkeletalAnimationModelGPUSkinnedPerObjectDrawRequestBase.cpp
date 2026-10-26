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

    // 描画申請の並びが変わるため、次のフレームで表を作り直す
    m_gpuDrivenTableBuilder.RequestRebuild();
}

bool FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView)
{
    // 1フレームに1回だけ、描画申請からインスタンスやMeshの表を更新してGPUへ送る
    // (表は持ち続けているため、変わった所だけが書き換わる)
    if (m_gpuDrivenDrawer.IsTableUploadRequired())
    {
        UpdateGPUDrivenTable(a_renderer);

        m_gpuDrivenDrawer.UploadFrameTable(a_renderer, m_gpuDrivenTableBuilder.EndTable());
    }

    return m_gpuDrivenDrawer.DispatchCulling(a_renderer, a_viewIndex, a_cullingView);
}

void FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::UpdateGPUDrivenTable(const Renderer& a_renderer)
{
    const auto& l_directCommandList          = a_renderer.GetREFDirectCommandList                         ();
    const auto& l_drawRequestDataElementList = m_drawRequestDataSmartPointerVectorList.GetREFElementDataList();

    m_gpuDrivenTableBuilder.ApplyTargetMaterialStaticTypeID(GetVALTargetMaterialStaticTypeID());
    m_gpuDrivenTableBuilder.BeginTable                     (l_drawRequestDataElementList.size());

    for (std::size_t l_drawRequestIndex = 0ULL; l_drawRequestIndex < l_drawRequestDataElementList.size(); ++l_drawRequestIndex)
    {
        const auto& l_drawRequestDataElement = l_drawRequestDataElementList[l_drawRequestIndex];
        const auto& l_drawRequestData        = l_drawRequestDataElement.m_type.lock();

        if (!l_drawRequestData)
        {
            m_gpuDrivenTableBuilder.SetInvalidInstance(l_drawRequestIndex);

            continue;
        }

        const auto& l_skeletalAnimationModelRecord = l_drawRequestData->m_skeletalAnimationModelRecord.lock();

        if (!l_skeletalAnimationModelRecord)
        {
            m_gpuDrivenTableBuilder.SetInvalidInstance(l_drawRequestIndex);

            continue;
        }

        // モデルのベイク済みアニメーションを取得し、まだGPUへ送っていなければコピー命令を積む
        const auto& l_bakedSkinning = FindOrCreateBakedSkinning(*l_skeletalAnimationModelRecord);

        if (!l_bakedSkinning)
        {
            m_gpuDrivenTableBuilder.SetInvalidInstance(l_drawRequestIndex);

            continue;
        }

        l_bakedSkinning->UploadIfNeeded(l_directCommandList);

        Struct::ModelGPUDrivenInstanceSource l_instanceSource = {};

        l_instanceSource.m_worldMatrix                 = &l_drawRequestData->m_worldMatrix;
        l_instanceSource.m_worldInverseTransposeMatrix = &l_drawRequestData->m_worldInverseTransposeMatrix;

        // 再生時刻から「何コマ目か」を決める
        l_instanceSource.m_animationFrameIndex = l_bakedSkinning->FetchVALFrameIndex(l_drawRequestData->m_motionIndex, l_drawRequestData->m_animationTimeSecond);

        // このインスタンスだけのモーフのウェイトがあれば、チャンネル数と範囲を合わせてから渡す
        if (!l_drawRequestData->m_morphWeightList.empty())
        {
            l_instanceSource.m_morphWeightList = &CreateMorphWeightList(*l_bakedSkinning, l_drawRequestData->m_morphWeightList);
        }

        m_gpuDrivenTableBuilder.SetInstance(l_drawRequestIndex,
                                            *l_skeletalAnimationModelRecord,
                                            l_instanceSource,
                                            &l_bakedSkinning->GetREFSkinningInfo());
    }
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

const std::vector<float>& FWK::Graphics::SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase::CreateMorphWeightList(const SkeletalAnimationBakedSkinning& a_bakedSkinning, const std::vector<float>& a_morphWeightList)
{
    const auto&         l_skinningInfo = a_bakedSkinning.GetREFSkinningInfo();
    const std::uint32_t l_channelCount = l_skinningInfo.m_morphChannelCount;

    // モーフを持たないモデルは、ウェイトを持たせない(空の配列を渡すと、ベイクしたウェイトを使う)
    if (l_channelCount == Constant::k_initialGPUDrivenSkinningInfoValue)
    {
        m_morphWeightWorkList.clear();

        return m_morphWeightWorkList;
    }

    // チャンネル数に合わせる(足りない分はウェイト0、多い分は使わない)
    // 毎回メモリを確保しないように、作業用の配列を使い回す
    m_morphWeightWorkList.assign(l_channelCount, Constant::k_initialModelMorphWeight);

    const std::size_t l_copyCount = std::min(a_morphWeightList.size(), m_morphWeightWorkList.size());

    for (std::uint32_t l_channelIndex = 0U; l_channelIndex < static_cast<std::uint32_t>(l_copyCount); ++l_channelIndex)
    {
        // エディターで設定したウェイトの範囲へ収める
        m_morphWeightWorkList[l_channelIndex] = a_bakedSkinning.ClampMorphWeight(l_channelIndex, a_morphWeightList[l_channelIndex]);
    }

    return m_morphWeightWorkList;
}