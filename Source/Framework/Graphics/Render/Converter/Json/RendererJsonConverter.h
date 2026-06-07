#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Converter
{
	class RendererJsonConverter final
	{
	public:

 		 RendererJsonConverter() = default;
		~RendererJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const;

		nlohmann::json Serialize(const Graphics::Renderer& a_renderer) const;

	private: 

		void DeserializeFrameResourceList(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const;
		void DeserializeSwapChain		 (const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const;
		
		nlohmann::json SerializeFrameResourceList(const Graphics::Renderer& a_renderer) const;
		nlohmann::json SerializeSwapChain		 (const Graphics::Renderer& a_renderer) const;
		
		static constexpr std::string_view k_frameResourceListJsonKey     = "FrameResourceList";
		static constexpr std::string_view k_frameResourceCountJsonKey    = "Count";
		static constexpr std::string_view k_frameResourceTemplateJsonKey = "Template";

		static constexpr std::string_view k_swapChainJsonKey = "SwapChain";

		static constexpr std::size_t k_defaultFrameResourceListCount = 4ULL;
		static constexpr std::size_t k_emptyFrameResourceListCount   = 0ULL;
	};
}