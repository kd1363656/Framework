#pragma once

namespace FWK::Graphics
{
	class RootSignature;
}

namespace FWK::Converter
{
	class RootSignatureJsonConverter final
	{
	public:

		struct RootParameterRecord final
		{
			// 使わない可能性を考慮してディスクリプタテーブル用のD3D12_DESCRIPTOR_RANGEをまとめたリストはポインタとして保持しておく
			// コピーコンストラクタ禁止を防ぐためにshared_ptrを使用
			std::shared_ptr<std::vector<D3D12_DESCRIPTOR_RANGE>> m_descriptorRangeList = nullptr;

			D3D12_ROOT_PARAMETER m_rootParameter = {};
 		};

	public:

		 RootSignatureJsonConverter() = default;
		~RootSignatureJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const;

		nlohmann::json Serialize(const Graphics::RootSignature& a_rootSignature) const;

		static constexpr UINT k_invalidRootParameterIndex = UINT_MAX;

	private:

		void DeserializeRootParameterIndexMap(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const;
		void DeserializeRootParameterList    (const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const;
		void DeserializeStaticSamplerDescList(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const;

		nlohmann::json SerializeRootParameterIndexMap(const Graphics::RootSignature& a_rootSignature) const;
		nlohmann::json SerializeRootParameterList    (const Graphics::RootSignature& a_rootSignature) const;
		nlohmann::json SerializeStaticSamplerDescList(const Graphics::RootSignature& a_rootSignature) const;

		void DeserializeDescriptorRangeList(const nlohmann::json& a_rootJson, RootParameterRecord& a_rootParameterRecord) const;

		nlohmann::json SerializeDescriptorRangeList(const RootParameterRecord& a_rootParameterRecord) const;

		// RootSignature
		static constexpr std::string_view k_rootParameterIndexMapJsonKey = "RootParameterIndexMap";
		static constexpr std::string_view k_rootParameterListJsonKey     = "RootParameterList";
		static constexpr std::string_view k_staticSamplerDescListJsonKey = "StaticSamplerDescList";
		static constexpr std::string_view k_rootSignatureFlagsJsonKey    = "RootSignatureFlags";
		static constexpr std::string_view k_rootSignatureVersionJsonKey  = "RootSignatureVersion";

		// RootParameterIndexMap
		static constexpr std::string_view k_rootParameterTypeJsonKey = "RootParameterType";
		static constexpr std::string_view k_indexJsonKey             = "Index";

		// RootParameter
		static constexpr std::string_view k_parameterTypeJsonKey       = "ParameterType";
		static constexpr std::string_view k_shaderVisibilityJsonKey    = "ShaderVisibility";
		static constexpr std::string_view k_descriptorRangeListJsonKey = "DescriptorRangeList";
		static constexpr std::string_view k_shaderRegisterJsonKey      = "ShaderRegister";
		static constexpr std::string_view k_registerSpaceJsonKey       = "RegisterSpace";
		static constexpr std::string_view k_num32BitValuesJsonKey      = "Num32BitValues";

		// StaticSamplerDesc
		static constexpr std::string_view k_filterJsonKey         = "Filter";
		static constexpr std::string_view k_addressUJsonKey       = "AddressU";
		static constexpr std::string_view k_addressVJsonKey       = "AddressV";
		static constexpr std::string_view k_addressWJsonKey       = "AddressW";
		static constexpr std::string_view k_mipLODBiasJsonKey     = "MipLODBias";
		static constexpr std::string_view k_maxAnisotropyJsonKey  = "MaxAnisotropy";
		static constexpr std::string_view k_comparisonFUNCJsonKey = "ComparisonFunc";
		static constexpr std::string_view k_borderColorJsonKey    = "BorderColor";
		static constexpr std::string_view k_minLODJsonKey         = "MinLOD";
		static constexpr std::string_view k_maxLODJsonKey         = "MaxLOD";

		// DescriptorRange
		static constexpr std::string_view k_rangeTypeJsonKey                         = "RangeType";
		static constexpr std::string_view k_numDescriptorsJsonKey                    = "NumDescriptors";
		static constexpr std::string_view k_baseShaderRegisterJsonKey                = "BaseShaderRegister";
		static constexpr std::string_view k_offsetInDescriptorsFromTableStartJsonKey = "OffsetInDescriptorsFromTableStart";

		static constexpr FLOAT k_defaultStaticSamplerMIPLODBias = 0.0F;
		static constexpr FLOAT k_defaultMINLOD				    = 0.0F;

		static constexpr UINT k_defaultRootConstantsNum32BitValues = 0U;

		static constexpr UINT k_defaultStaticSamplerMAXAnisotropy = 1U;

		static constexpr UINT k_defaultShaderRegister = 0U;
		static constexpr UINT k_defaultRegisterSpace  = 0U;

		static constexpr UINT k_defaultDescriptorRangeNUMDescriptors = 0U;
		static constexpr UINT k_defaultBaseShaderRegister            = 0U;
		static constexpr UINT k_invalidNUMDescriptorRange            = 0U;
	};
}