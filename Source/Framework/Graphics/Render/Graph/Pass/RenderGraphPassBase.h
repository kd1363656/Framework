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

				 RenderGraphPassBase() = default;
		virtual ~RenderGraphPassBase() = default;

		RenderGraphPassBase(const RenderGraphPassBase&)  = delete;
		RenderGraphPassBase(	  RenderGraphPassBase&&) = delete;

		RenderGraphPassBase& operator=(const RenderGraphPassBase&)  = delete;
		RenderGraphPassBase& operator=(	     RenderGraphPassBase&&) = delete;

		virtual void Execute(const ResourceContext& a_resourceContext, const FrameResource& a_frameResource, const Renderer& a_renderer) = 0;

		const auto& GetREFResourceAccessList() const { return m_resourceAccessList; }

	protected:

		void ReadResource(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphResourceUsage a_usage);

		void WriteResource(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphResourceUsage a_usage);

	private:

		void AddResourceAccess(const Enum::RenderGraphResourceType a_resourceType, const Enum::RenderGraphAccessType a_accessType, const Enum::RenderGraphResourceUsage a_usage);

		std::vector<Struct::RenderGraphResourceAccess> m_resourceAccessList = {};

		FWK_DEFINE_TYPE_INFO_ROOT(RenderGraphPassBase)
	};
}