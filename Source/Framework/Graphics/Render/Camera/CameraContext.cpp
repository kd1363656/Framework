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

    // カリング用カメラ(どのカメラの視錐台でカリングするか)を決めて、カリング用の定数バッファの参照先を差し替える
    // 描画用のカメラとは別の定数バッファ(b4)なので、
    // カリングだけシーンカメラ基準にしても、画面に映る範囲(ViewProjectionMatrix)は描画用カメラのままになる
    if (const auto& l_cullingCameraPassDrawRequest = a_renderGraph.FindVALDrawRequestPass<CullingCameraPassDrawRequest>().lock();
        l_cullingCameraPassDrawRequest)
    {
        l_cullingCameraPassDrawRequest->SetSourceCamera(FetchVALCullingCamera());
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
std::weak_ptr<FWK::Struct::CBCameraPass> FWK::Graphics::CameraContext::FetchVALCullingCamera() const
{
    // カリングを描画用とは別のカメラで行うのは、エディターのデバッグ表示中だけ
    // デバッグモードはエディターが表示されているときにだけなるため、ゲーム実行時はここに入らない
    // デバッグモードで、カリング結果の確認がONで、シーンカメラが使えるなら、シーンカメラでカリングする
    // 描画はエディターカメラのまま、「シーンカメラに映る物だけが描かれる」様子をエディターカメラから確認できる
    if (m_viewMode == Enum::CameraViewMode::Debug &&
        m_isCullingBySceneCamera && 
        !m_sceneCamera.expired())
    {
        return m_sceneCamera; 
    }

    // それ以外は、描画に使うカメラでカリングする(今までと同じ動き)
    return FetchVALViewCamera();
}