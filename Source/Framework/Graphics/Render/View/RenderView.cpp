#include "RenderView.h"

bool FWK::Graphics::RenderView::ApplyViewCamera(const RenderGraph& a_renderGraph, ShadowContext& a_shadowContext) const
{
    // このビューが描画に使うカメラを、このビューのCameraContextに決めてもらい、
    // 描画用の定数バッファとCascade計算へ反映する
    // CameraContextはビューごとに1つずつ持つ
    // そのためビューが増えても、ビューごとに別のカメラを使うことができる
    // 使えるカメラが無かった場合はfalseが返るため、そのまま呼び出し側へ返す
    return m_cameraContext.ApplyViewCamera(a_renderGraph, a_shadowContext);
}