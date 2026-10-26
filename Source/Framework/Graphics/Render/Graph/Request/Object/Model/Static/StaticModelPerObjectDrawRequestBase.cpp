#include "StaticModelPerObjectDrawRequestBase.h"

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::BeginFrame()
{
    // 参照が途切れているstd::weak_ptrを削除する
    m_forwardDrawRequestDataSmartPointerVectorList.RemoveExpiredElements();

    // 新しいフレームの準備(インスタンスの表を送り直す、Viewのカリング状態を戻す)
    m_gpuDrivenDrawer.BeginFrame();

    m_currentDrawViewIndex = k_defaultDrawViewIndex;
}

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource&)
{
    // 以前はここで「全モデル × 全Mesh」をCPUでループし、1つずつ定数バッファを作ってDispatchMeshしていた
    // 現在はDispatchGPUCullingでGPUが作った描画コマンドを、1回の命令でまとめて実行する
    m_gpuDrivenDrawer.ExecuteIndirect(a_renderer, a_rootSignature, m_currentDrawViewIndex);
}

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::AddDrawRequest(const std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData>& a_drawRequestData)
{
    FWK_ASSERT_RETURN_IF(!a_drawRequestData, "DrawRequestDataが無効のため、描画申請の追加が出来ませんでした。");

    m_forwardDrawRequestDataSmartPointerVectorList.Add(a_drawRequestData);

    // 描画申請の並びが変わるため、次のフレームで表を作り直す
    m_gpuDrivenTableBuilder.RequestRebuild();
}

bool FWK::Graphics::StaticModelPerObjectDrawRequestBase::DispatchGPUCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView)
{
    // 1フレームに1回だけ、描画申請からインスタンスやMeshの表を更新してGPUへ送る
    // (表は持ち続けているため、変わった所だけが書き換わる)
    if (m_gpuDrivenDrawer.IsTableUploadRequired())
    {
        UpdateGPUDrivenTable();

        m_gpuDrivenDrawer.UploadFrameTable(a_renderer, m_gpuDrivenTableBuilder.EndTable());
    }

    return m_gpuDrivenDrawer.DispatchCulling(a_renderer, a_viewIndex, a_cullingView);
}

void FWK::Graphics::StaticModelPerObjectDrawRequestBase::UpdateGPUDrivenTable()
{
    const auto& l_drawRequestDataElementList = m_forwardDrawRequestDataSmartPointerVectorList.GetREFElementDataList();

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

        const auto& l_staticModelRecord = l_drawRequestData->m_staticModelRecord.lock();

        if (!l_staticModelRecord)
        {
            m_gpuDrivenTableBuilder.SetInvalidInstance(l_drawRequestIndex);

            continue;
        }

        Struct::ModelGPUDrivenInstanceSource l_instanceSource = {};

        l_instanceSource.m_worldMatrix                 = &l_drawRequestData->m_worldMatrix;
        l_instanceSource.m_worldInverseTransposeMatrix = &l_drawRequestData->m_worldInverseTransposeMatrix;

        // StaticModelはスキニングしないため、追加情報は無し
        m_gpuDrivenTableBuilder.SetInstance(l_drawRequestIndex,
                                            *l_staticModelRecord,
                                            l_instanceSource,
                                            nullptr);
    }
}