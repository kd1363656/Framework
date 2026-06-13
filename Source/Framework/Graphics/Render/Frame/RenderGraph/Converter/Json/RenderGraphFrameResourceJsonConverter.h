#pragma once

namespace FWK::Graphics
{
	class RenderGraphFrameResource;
}

namespace FWK::Converter
{
	class RenderGraphFrameResourceJsonConverter final
	{
	public:

		 RenderGraphFrameResourceJsonConverter() = default;
		~RenderGraphFrameResourceJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const;

		nlohmann::json Serialize(const Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const;

	private:

		void DeserializeRenderTargetPassTextureList(const nlohmann::json& a_rootJson, Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const;

		nlohmann::json SerializeRenderTargetPassTextureList(const Graphics::RenderGraphFrameResource& a_renderGraphFrameResource) const;

		static constexpr std::string_view k_renderTargetPassTextureListJsonKey = "RenderTargetPassTextureList";
	};
}