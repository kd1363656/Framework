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

		static constexpr std::string_view k_formatJsonKey				       = "Format";
		static constexpr std::string_view k_depthClearValueJsonKey		       = "DepthClearValue";
		static constexpr std::string_view k_stencilClearValueJsonKey	       = "StencilClearValue";
		static constexpr std::string_view k_renderGraphDepthStencilTypeJsonKey = "RenderGraphDepthStencilType";
		static constexpr std::string_view k_widthJsonKey				       = "Width";
		static constexpr std::string_view k_heightJsonKey				       = "Height";
		static constexpr std::string_view k_isFixedSizeJsonKey			       = "IsFixedSize";
		static constexpr std::string_view k_isSkipClearOnBeginFrameJsonKey     = "IsSkipClearOnBeginFrame";
	};
}