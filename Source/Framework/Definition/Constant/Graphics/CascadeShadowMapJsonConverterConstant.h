#pragma once

namespace FWK::Constant
{
	inline constexpr DXGI_FORMAT k_cascadeShadowMapDefaultResourceFormat = DXGI_FORMAT_R32_TYPELESS;
	inline constexpr DXGI_FORMAT k_cascadeShadowMapDefaultDSVFormat      = DXGI_FORMAT_D32_FLOAT;
	inline constexpr DXGI_FORMAT k_cascadeShadowMapDefaultSRVFormat      = DXGI_FORMAT_R32_FLOAT;

	inline constexpr UINT k_cascadeShadowMapDefaultResolution = 2048U;
	
	inline constexpr UINT16 k_cascadeShadowMapDefaultMAXCascadeCount = 4U;
}