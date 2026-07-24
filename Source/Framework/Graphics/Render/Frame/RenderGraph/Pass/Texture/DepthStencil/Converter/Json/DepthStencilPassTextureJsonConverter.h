#pragma once

namespace FWK::Graphics
{
	class DepthStencilPassTexture;
}

namespace FWK::Converter
{
	class DepthStencilPassTextureJsonConverter final
	{
	public:

		 DepthStencilPassTextureJsonConverter() = default;
		~DepthStencilPassTextureJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::DepthStencilPassTexture& a_depthStencilPassTexture) const;

		nlohmann::json Serialize(const Graphics::DepthStencilPassTexture& a_depthStencilPassTexture) const;

	private:

		static constexpr std::string_view k_resourceFormatJsonKey              = "ResourceFormat";
		static constexpr std::string_view k_dsvFormatJsonKey                   = "DSVFormat";
		static constexpr std::string_view k_srvFormatJsonKey                   = "SRVFormat";
		static constexpr std::string_view k_depthClearValueJsonKey             = "DepthClearValue";
		static constexpr std::string_view k_arraySizeJsonKey                   = "ArraySize";
		static constexpr std::string_view k_mipLevelsJsonKey                   = "MIPLevels";
		static constexpr std::string_view k_sampleCountJsonKey                 = "SampleCount";
		static constexpr std::string_view k_sampleQualityJsonKey               = "SampleQuality";
		static constexpr std::string_view k_stencilClearValueJsonKey           = "StencilClearValue";
		static constexpr std::string_view k_renderGraphDepthStencilTypeJsonKey = "RenderGraphDepthStencilType";
		static constexpr std::string_view k_widthJsonKey                       = "Width";
		static constexpr std::string_view k_heightJsonKey                      = "Height";
		static constexpr std::string_view k_isFixedSizeJsonKey                 = "IsFixedSize";
	};
}