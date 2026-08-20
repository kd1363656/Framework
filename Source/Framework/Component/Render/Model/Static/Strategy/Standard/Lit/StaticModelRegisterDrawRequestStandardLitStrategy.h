#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStandardLitStrategy final : public StaticModelRegisterDrawRequestStrategyBase
	{
	public:

		 StaticModelRegisterDrawRequestStandardLitStrategy()          = default;
		~StaticModelRegisterDrawRequestStandardLitStrategy() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestStandardLitStrategy, StaticModelRegisterDrawRequestStrategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory, FWK::StaticModelRegisterDrawRequestStandardLitStrategy)