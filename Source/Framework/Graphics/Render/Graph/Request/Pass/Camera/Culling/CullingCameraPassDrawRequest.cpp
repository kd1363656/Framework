#include "CullingCameraPassDrawRequest.h"

bool FWK::Graphics::CullingCameraPassDrawRequest::SetupPassConstantBuffer(const RootSignature& a_rootSignature, const DirectCommandList& a_directCommandList, const FrameResource& a_frameResource)
{
    // 定数バッファの更新
    UpdateConstantBuffer();

    SetupConstantBuffer<CullingCameraPassDynamicConstantBufferUploader>(m_constantBuffer,
                                                                        a_rootSignature,
                                                                        a_directCommandList,
                                                                        a_frameResource,
                                                                        Enum::RootParameterType::CBCullingCameraPass);

    return true;
}

void FWK::Graphics::CullingCameraPassDrawRequest::UpdateConstantBuffer()
{
    const auto& l_sourceCamera = m_sourceCamera.lock();

    if (!l_sourceCamera) { return; }

    // 描画用カメラの定数バッファから、カリングに必要な値だけを写す
    m_constantBuffer.m_viewMatrix          = l_sourceCamera->m_viewMatrix;
    m_constantBuffer.m_nearClip            = l_sourceCamera->m_nearClip;
    m_constantBuffer.m_farClip             = l_sourceCamera->m_farClip;
    m_constantBuffer.m_tanHalfFOVX         = l_sourceCamera->m_tanHalfFOVX;
    m_constantBuffer.m_tanHalfFOVY         = l_sourceCamera->m_tanHalfFOVY;
    m_constantBuffer.m_cameraWorldPosition = l_sourceCamera->m_cameraWorldPosition;
}