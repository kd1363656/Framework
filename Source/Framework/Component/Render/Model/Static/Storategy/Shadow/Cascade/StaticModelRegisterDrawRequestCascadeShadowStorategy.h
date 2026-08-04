#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestCascadeShadowStorategy final : public StaticModelRegisterDrawRequestStorategyBase
	{
	public:

		 StaticModelRegisterDrawRequestCascadeShadowStorategy()          = default;
		~StaticModelRegisterDrawRequestCascadeShadowStorategy() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestCascadeShadowStorategy, StaticModelRegisterDrawRequestStorategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::StaticModelRegisterDrawRequestStorategyBaseUniqueFactory, FWK::StaticModelRegisterDrawRequestCascadeShadowStorategy)