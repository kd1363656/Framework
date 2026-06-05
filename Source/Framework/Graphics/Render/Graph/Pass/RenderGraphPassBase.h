#pragma once

namespace FWK::Graphics
{
	class Renderer;
}

namespace FWK::Graphics
{
	class RenderGraphPassBase
	{
	public:

		explicit RenderGraphPassBase(const Enum::RenderGraphPassType a_renderGraphPassType);
		virtual ~RenderGraphPassBase();

		virtual void Execute(const ResourceContext& a_resourceContext, Renderer& a_renderer) { /*必要に応じてオーバーライドしてください*/ };

		const auto& GetREFResourceAccessList() const { return m_resourceAccessList; }

		auto GetVALRenderGraphPassType() const { return m_renderGraphPassType; }

	protected:

		void AddResourceAccess(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphResourceAccessType a_accessType);

	private:

		std::vector<Struct::RenderGraphResourceAccess> m_resourceAccessList = {};

		Enum::RenderGraphPassType m_renderGraphPassType = Enum::RenderGraphPassType::None;
	};
}