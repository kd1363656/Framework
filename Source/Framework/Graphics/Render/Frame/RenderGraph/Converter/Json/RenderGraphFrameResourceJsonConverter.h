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
	};
}