#pragma once

namespace FWK::Graphics
{
	class CascadeShadowMap;
}

namespace FWK::Converter
{
	class CascadeShadowMapJsonConverter
	{
	public:
		 CascadeShadowMapJsonConverter() = default;
		~CascadeShadowMapJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::CascadeShadowMap& a_cascadeShadowMap) const;

		nlohmann::json Serialize(const Graphics::CascadeShadowMap& a_cascadeShadowMap) const;

	private:

		static constexpr std::string_view k_resourceFormatJsonKey    = "ResourceFormat";
		static constexpr std::string_view k_dsvFormatJsonKey         = "DSVFormat";
		static constexpr std::string_view k_srvFormatJsonKey         = "SRVFormat";
		static constexpr std::string_view k_depthClearValueJsonKey   = "DepthClearValue";
		static constexpr std::string_view k_stencilClearValueJsonKey = "StencilClearValue";

		static constexpr std::string_view k_cascadeCountJsonKey = "CascadeCount";
		static constexpr std::string_view k_mipLevelsJsonKey    = "MIPLevels";

		static constexpr std::string_view k_sampleCountJsonKey   = "SampleCount";
		static constexpr std::string_view k_sampleQualityJsonKey = "SampleQuality";

		static constexpr std::string_view k_resolutionJsonKey = "Resolution";
	};
}