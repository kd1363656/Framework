#include "CameraPassDrawRequest.h"

bool FWK::Graphics::CameraPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
    // 定数バッファの更新(描画カメラの値を写し、カリングカメラの値を書き足す)
    UpdateCameraConstantBuffer();

    SetupConstantBuffer<CameraPassDynamicConstantBufferUploader>(GetREFConstantBuffer(),
                                                                 a_rootSignature,
                                                                 a_directCommandList,
                                                                 a_frameResource,
                                                                 Enum::RootParameterType::CBCameraPass);

    return true;
}

std::shared_ptr<FWK::Struct::CBCameraPass> FWK::Graphics::CameraPassDrawRequest::FetchVALDrawConstantBuffer() const
{
    if (const auto& l_overrideSource = m_overrideSourceConstantBuffer.lock();
        l_overrideSource)
    {
        return l_overrideSource;
    }

    return GetREFSourceConstantBuffer().lock();
}
std::shared_ptr<FWK::Struct::CBCameraPass> FWK::Graphics::CameraPassDrawRequest::FetchVALCullingConstantBuffer() const
{
    if (const auto& l_cullingSource = m_cullingSourceConstantBuffer.lock();
        l_cullingSource)
    {
        return l_cullingSource;
    }

    return FetchVALDrawConstantBuffer();
}

void FWK::Graphics::CameraPassDrawRequest::UpdateCameraConstantBuffer()
{
    const auto& l_drawSource = FetchVALDrawConstantBuffer();

    if (!l_drawSource) { return; }

    auto& l_constantBuffer = GetMutableREFConstantBuffer();

    l_constantBuffer = *l_drawSource;

    // カリングカメラの値は、描画カメラの定数バッファには入っていないためここで書き足す
    // (通常は描画カメラと同じ値になる)
    const auto& l_cullingSource = FetchVALCullingConstantBuffer();

    if (!l_cullingSource) { return; }

    l_constantBuffer.m_cullingViewMatrix          = l_cullingSource->m_viewMatrix;
    l_constantBuffer.m_cullingNearClip            = l_cullingSource->m_nearClip;
    l_constantBuffer.m_cullingFarClip             = l_cullingSource->m_farClip;
    l_constantBuffer.m_cullingTanHalfFOVX         = l_cullingSource->m_tanHalfFOVX;
    l_constantBuffer.m_cullingTanHalfFOVY         = l_cullingSource->m_tanHalfFOVY;
    l_constantBuffer.m_cullingCameraWorldPosition = l_cullingSource->m_cameraWorldPosition;

    l_constantBuffer.m_isCullingDebugVisualize = m_isCullingDebugVisualize ? Constant::k_cameraPassCullingDebugVisualizeOn : Constant::k_cameraPassCullingDebugVisualizeOff;
}
