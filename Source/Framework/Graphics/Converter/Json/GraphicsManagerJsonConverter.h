#pragma once

namespace FWK::Graphics
{
	class GraphicsManager;
}

namespace FWK::Converter
{
	class GraphicsManagerJsonConverter final
	{
	public:

		 GraphicsManagerJsonConverter() = default;
		~GraphicsManagerJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::GraphicsManager& a_graphicsManager) const;

		nlohmann::json Serialize(const Graphics::GraphicsManager& a_graphicsManager) const;

	private:

		static constexpr std::string_view k_rendererJsonKey = "Renderer";
	};
}