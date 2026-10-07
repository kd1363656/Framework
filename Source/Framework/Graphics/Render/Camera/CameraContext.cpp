#include "CameraContext.h"

bool FWK::Graphics::CameraContext::ApplyViewCamera(const RenderGraph& a_renderGraph, ShadowContext& a_shadowContext) const
{
    // 現在のモードに合わせて、描画に使うカメラの定数バッファを決める
    const auto& l_viewCamera = FetchVALViewCamera().lock();

    // 使えるカメラが1つもない場合は、これまでの登録をそのままにする
    // 呼び出し側が「このビューは描画しない」と判断できるように、falseを返す
    if (!l_viewCamera) { return false; }

    // 描画用カメラの定数バッファ参照先を、決めたカメラへ差し替える
    if (const auto& l_cameraPassDrawRequest = a_renderGraph.FindVALDrawRequestPass<CameraPassDrawRequest>().lock();
        l_cameraPassDrawRequest)
    {
        l_cameraPassDrawRequest->SetSourceConstantBuffer(l_viewCamera);
    }

    // Cascade計算に使用するカメラも、描画に使うカメラへ合わせる
    auto& l_cascadeShadowMap = a_shadowContext.GetMutableREFCascadeShadowMap();

    l_cascadeShadowMap.SetCBCameraPass(l_viewCamera);

    return true;
}

std::weak_ptr<FWK::Struct::CBCameraPass> FWK::Graphics::CameraContext::FetchVALViewCamera() const
{
    // デバッグモードではデバッグカメラを優先し、シーンモードではシーンカメラを優先する
    const bool l_isDebugMode = m_viewMode == Enum::CameraViewMode::Debug;

    const auto& l_preferredCamera = l_isDebugMode ? m_debugCamera : m_sceneCamera;
    const auto& l_fallbackCamera  = l_isDebugMode ? m_sceneCamera : m_debugCamera;

    // 優先するカメラがまだ有効なら、そのカメラを使う
    if (!l_preferredCamera.expired()) { return l_preferredCamera; }

    // 優先するカメラが無い、または消えている場合は、もう片方のカメラを使う
    return l_fallbackCamera;
}