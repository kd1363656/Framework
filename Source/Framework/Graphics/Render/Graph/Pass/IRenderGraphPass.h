#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	class IRenderGraphPass
	{
	public:

				 IRenderGraphPass() = default;
		virtual ~IRenderGraphPass() = default;

		virtual Enum::RenderGraphPassType GetVALRenderGraphPassType() const = 0;

		virtual const std::vector<Struct::RenderGraphResourceAccess>& GetREFResourceAccessList() const = 0;

		virtual void PostCreateSetup(Renderer&) { /*必要に応じてオーバーライドしてください*/ };

		virtual void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer) = 0;
	};
}