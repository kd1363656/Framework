#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestCascadeShadowStrategy final : public StaticModelRegisterDrawRequestStrategyBase
	{
	public:

		 StaticModelRegisterDrawRequestCascadeShadowStrategy()          = default;
		~StaticModelRegisterDrawRequestCascadeShadowStrategy() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestCascadeShadowStrategy, StaticModelRegisterDrawRequestStrategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory, FWK::StaticModelRegisterDrawRequestCascadeShadowStrategy)