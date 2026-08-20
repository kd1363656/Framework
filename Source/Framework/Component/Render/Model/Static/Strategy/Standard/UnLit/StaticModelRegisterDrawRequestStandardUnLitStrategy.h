#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStandardUnLitStrategy final : public StaticModelRegisterDrawRequestStrategyBase
	{
	public:

		 StaticModelRegisterDrawRequestStandardUnLitStrategy()          = default;
		~StaticModelRegisterDrawRequestStandardUnLitStrategy() override = default;

		void Execute(StaticModelComponent& a_staticModelComponent) override;
		
		FWK_DEFINE_TYPE_INFO(StaticModelRegisterDrawRequestStandardUnLitStrategy, StaticModelRegisterDrawRequestStrategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory, FWK::StaticModelRegisterDrawRequestStandardUnLitStrategy)