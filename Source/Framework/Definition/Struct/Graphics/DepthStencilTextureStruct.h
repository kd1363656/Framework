#pragma once

namespace FWK::Struct
{
    struct DepthStencilTextureSettings
    {
        DXGI_FORMAT m_resourceFormat = Constant::k_defaultDepthStencilResourceFormat;
        DXGI_FORMAT m_dsvFormat      = Constant::k_defaultDepthStencilDSVFormat;
        DXGI_FORMAT m_srvFormat      = Constant::k_defaultDepthStencilSRVFormat;

        FLOAT m_depthClearValue = Constant::k_defaultDepthClearValue;

        UINT16 m_arraySize = Constant::k_defaultDepthStencilArraySize;
        UINT16 m_mipLevels = Constant::k_defaultDepthStencilMipLevels;

        UINT m_sampleCount   = Constant::k_defaultSampleCount;
        UINT m_sampleQuality = Constant::k_defaultSampleQuality;

        UINT8 m_stencilClearValue = Constant::k_defaultStencilClearValue;
    };
}