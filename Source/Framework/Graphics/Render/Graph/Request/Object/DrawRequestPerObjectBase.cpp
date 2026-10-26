#include "DrawRequestPerObjectBase.h"

bool FWK::Graphics::DrawRequestPerObjectBase::SetupModelLODSelector(const Renderer& a_renderer, const float a_maxScreenSpaceErrorPixel, ModelLODSelector& a_modelLODSelector) const
{
    const auto& l_renderGraph           = a_renderer.GetREFRenderGraph                                ();
    const auto& l_cameraPassDrawRequest = l_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_cameraPassDrawRequest, "CameraPassDrawRequestが無効のため、LOD選択の準備に失敗しました。", false);

    // LODは「カリングに使うカメラ」からの距離で選ぶ(GPUドリブン描画のLOD選択と同じ基準にする)
    const auto& l_cbCameraPass = l_cameraPassDrawRequest->FetchVALCullingConstantBuffer();

    // カメラが登録されていない場合はLODを選べないため、呼び出し側で元の形(全Meshlet)を描画する
    if (!l_cbCameraPass) { return false; }

    // 画面の高さ(ピクセル)は、画面描画用のViewportから取得する
    const auto& l_screenRenderArea = a_renderer.GetREFScreenRenderArea();
    const auto& l_viewport         = l_screenRenderArea.GetREFViewport();

    a_modelLODSelector.Setup(*l_cbCameraPass, l_viewport.Height, a_maxScreenSpaceErrorPixel);

    return true;
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::DrawRequestPerObjectBase::FetchTextureSRVDescriptorIndex(const std::weak_ptr<TextureRecord>& a_textureRecord) const
{
    const auto& l_textureRecord = a_textureRecord.lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_textureRecord, "TextureRecordが無効なため、TextureSRVDescriptorIndexの取得に失敗しました。", DescriptorHeap::k_invalidDescriptorIndex);

    return l_textureRecord->GetVALSRVDescriptorIndex();
}