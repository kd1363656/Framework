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
		void DeserializeRootSignatureMap (const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const;
		void DeserializePipelineStateMap (const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const;
		
		nlohmann::json SerializeFrameResourceList(const Graphics::Renderer& a_renderer) const;
		nlohmann::json SerializeRootSignatureMap (const Graphics::Renderer& a_renderer) const;
		nlohmann::json SerializePipelineStateMap (const Graphics::Renderer& a_renderer) const;
		
		static constexpr std::string_view k_frameResourceListJsonKey     = "FrameResourceList";
		static constexpr std::string_view k_frameResourceCountJsonKey    = "Count";
		static constexpr std::string_view k_frameResourceTemplateJsonKey = "Template";

		static constexpr std::string_view k_swapChainJsonKey = "SwapChain";

		static constexpr std::string_view k_rootSignatureMapJsonKey = "RootSignatureMap";
		static constexpr std::string_view k_pipelineStateMapJsonKey = "PipelineStateMap";

		static constexpr std::string_view k_rootSignatureTypeJsonKey = "RootSignatureType";
		static constexpr std::string_view k_rootSignatureJsonKey     = "RootSignature";
		
		static constexpr std::string_view k_pipelineStateTypeJsonKey = "PipelineStateType";
		static constexpr std::string_view k_pipelineStateJsonKey     = "PipelineState";

		static constexpr std::string_view k_renderGraphJsonKey = "RenderGraph";

		static constexpr std::size_t k_defaultFrameResourceListCount = 4ULL;
		static constexpr std::size_t k_emptyFrameResourceListCount   = 0ULL;
	};
}