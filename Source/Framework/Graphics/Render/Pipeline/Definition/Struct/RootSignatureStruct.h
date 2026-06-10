#pragma once

namespace FWK::Struct
{
	struct RootParameterCreateDesc final
	{
		Enum::RootParameterSlot m_rootParameterSlot = Enum::RootParameterSlot::Count;

		D3D12_ROOT_PARAMETER_TYPE m_parameterType    = D3D12_ROOT_PARAMETER_TYPE_CBV;
		D3D12_SHADER_VISIBILITY   m_shaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		UINT m_shaderRegister = 0U;
		UINT m_registerSpace  = 0U;
	};

	struct StaticSamplerCreateDesc final
	{
		D3D12_FILTER m_filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;

		D3D12_TEXTURE_ADDRESS_MODE m_addressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		D3D12_TEXTURE_ADDRESS_MODE m_addressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		D3D12_TEXTURE_ADDRESS_MODE m_addressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

		D3D12_COMPARISON_FUNC     m_comparisonFUNC = D3D12_COMPARISON_FUNC_ALWAYS;
		D3D12_STATIC_BORDER_COLOR m_borderColor    = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

		D3D12_SHADER_VISIBILITY m_shaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		float m_mipLODBias = 0.0F;
		float m_minLOD     = 0.0F;
		float m_maxLOD     = D3D12_FLOAT32_MAX;

		UINT m_shaderRegister = 0U;
		UINT m_registerSpace  = 0U;
		UINT m_maxAnisotropy  = 1U;
	};

	struct RootSignatureCreateDesc final
	{
		D3D_ROOT_SIGNATURE_VERSION m_rootSignatureVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		D3D12_ROOT_SIGNATURE_FLAGS m_rootSignatureFlags   = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		std::vector<RootParameterCreateDesc> m_rootParameterCreateDescList = {};
		std::vector<StaticSamplerCreateDesc> m_staticSamplerCreateDescList = {};
	};
}