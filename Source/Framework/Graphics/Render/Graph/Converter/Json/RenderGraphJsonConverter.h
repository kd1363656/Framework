#pragma once

namespace FWK::Graphics
{
	class RenderGraph;
}

namespace FWK::Converter
{
	class RenderGraphJsonConverter final
	{
	public:

		 RenderGraphJsonConverter() = default;
		~RenderGraphJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const;

		nlohmann::json Serialize(const Graphics::RenderGraph& a_renderGraph) const;

	private:

		void DeserializePassList(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_renderGraph) const;

		nlohmann::json SerializePassList(const Graphics::RenderGraph& a_renderGraph) const;

		static constexpr std::string_view k_renderGraphPassListJsonKey     = "RenderGraphPassList";
		static constexpr std::string_view k_renderGraphPassTypeNameJsonKey = "RenderGraphPassTypeName";
	};
}