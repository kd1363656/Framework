#pragma once

namespace FWK::Struct
{
	struct DepthStencilPassTextureStruct
	{
		static constexpr DXGI_FORMAT k_defaultResourceFormat = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT k_defaultDSVFormat      = DXGI_FORMAT_D32_FLOAT;
		static constexpr DXGI_FORMAT k_defaultSRVFormat      = DXGI_FORMAT_UNKNOWN;
	
		static constexpr UINT16 k_defaultArraySize = 1U;
		static constexpr UINT16 k_defaultMipLevels = 1U;

		DXGI_FORMAT m_resourceFormat = k_defaultResourceFormat;
		DXGI_FORMAT m_dsvFormat      = k_defaultDSVFormat;
		DXGI_FORMAT m_srvFormat      = k_defaultSRVFormat;

		FLOAT m_depthClearValue = Constant::k_defaultDepthClearValue;

		UINT16 m_arraySize = k_defaultArraySize;
		UINT16 m_mipLevels = k_defaultMipLevels;

		UINT m_sampleCount   = Constant::k_defaultSampleCount;
		UINT m_sampleQuality = Constant::k_defaultSampleQuality;

		UINT8 m_stencilClearValue = Constant::k_defaultStencilClearValue;
	};
}