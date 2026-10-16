#pragma once

namespace FWK::Struct
{
    struct DepthStencilTextureSettings
    {
        DXGI_FORMAT m_resourceFormat = Constant::k_defaultResourceFormat;
        DXGI_FORMAT m_dsvFormat      = Constant::k_defaultDSVFormat;
        DXGI_FORMAT m_srvFormat      = Constant::k_defaultSRVFormat;

        FLOAT m_depthClearValue = Constant::k_defaultDepthClearValue;

        UINT16 m_arraySize = Constant::k_defaultArraySize;
        UINT16 m_mipLevels = Constant::k_defaultMipLevels;

        UINT m_sampleCount   = Constant::k_defaultSampleCount;
        UINT m_sampleQuality = Constant::k_defaultSampleQuality;

        UINT8 m_stencilClearValue = Constant::k_defaultStencilClearValue;
    };
}