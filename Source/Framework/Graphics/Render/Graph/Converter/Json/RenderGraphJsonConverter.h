#pragma once

namespace FWK::Graphics
{
	class RenderGraph;
}

namespace FWK::Converter
{
	class RenderGraphJsonConverter
	{
	public:

		 RenderGraphJsonConverter() = default;
		~RenderGraphJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraph& a_swapChain) const;

		nlohmann::json Serialize(const Graphics::RenderGraph& a_swapChain) const;
	};
}