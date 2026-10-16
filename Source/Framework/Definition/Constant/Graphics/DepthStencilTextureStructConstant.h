#pragma once

namespace FWK::Constant
{
    inline constexpr DXGI_FORMAT k_defaultResourceFormat = DXGI_FORMAT_D32_FLOAT;
    inline constexpr DXGI_FORMAT k_defaultDSVFormat      = DXGI_FORMAT_D32_FLOAT;
    inline constexpr DXGI_FORMAT k_defaultSRVFormat      = DXGI_FORMAT_UNKNOWN;

    inline constexpr UINT16 k_defaultArraySize = 1U;
    inline constexpr UINT16 k_defaultMipLevels = 1U;
}