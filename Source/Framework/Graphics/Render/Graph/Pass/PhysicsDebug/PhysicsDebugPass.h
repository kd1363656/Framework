#pragma once

namespace FWK::Graphics
{
	class PhysicsDebugPass final : public RenderGraphPassBase
	{
	public:

		 PhysicsDebugPass();
		~PhysicsDebugPass() override;

		void Execute(Renderer& a_renderer, RenderGraph& a_renderGraph) override;

		FWK_DEFINE_TYPE_INFO(PhysicsDebugPass, RenderGraphPassBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::RenderGraphPassUniqueFactory, FWK::Graphics::PhysicsDebugPass)