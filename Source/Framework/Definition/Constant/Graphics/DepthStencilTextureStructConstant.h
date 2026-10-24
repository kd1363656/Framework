#pragma once

namespace FWK::Constant
{
    inline constexpr DXGI_FORMAT k_defaultDepthStencilResourceFormat = DXGI_FORMAT_D32_FLOAT;
    inline constexpr DXGI_FORMAT k_defaultDepthStencilDSVFormat      = DXGI_FORMAT_D32_FLOAT;
    inline constexpr DXGI_FORMAT k_defaultDepthStencilSRVFormat      = DXGI_FORMAT_UNKNOWN;

    inline constexpr UINT16 k_defaultDepthStencilArraySize = 1U;
    inline constexpr UINT16 k_defaultDepthStencilMipLevels = 1U;
}