#pragma once

namespace FWK::Graphics
{
	class RenderTargetPassTexture;
}

namespace FWK::Converter
{
	class RenderTargetPassTextureJsonConverter
	{
	public:

		 RenderTargetPassTextureJsonConverter() = default;
		~RenderTargetPassTextureJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderTargetPassTexture& a_renderTargetPassTexture) const;

		nlohmann::json Serialize(const Graphics::RenderTargetPassTexture& a_renderTargetPassTexture) const;

	private:

		static constexpr std::string_view k_clearColorJsonKey			   = "ClearColor";
		static constexpr std::string_view k_formatJsonKey				   = "Format";
		static constexpr std::string_view k_renderGraphResourceTypeJsonKey = "RenderGraphResourceType";
		static constexpr std::string_view k_widthJsonKey				   = "Width";
		static constexpr std::string_view k_heightJsonKey				   = "Height";
		static constexpr std::string_view k_isFixedSizeJsonKey			   = "IsFixedSize";
		static constexpr std::string_view k_isSkipClearOnBeginFrameJsonKey = "IsSkipClearOnBeginFrame";
	};
}