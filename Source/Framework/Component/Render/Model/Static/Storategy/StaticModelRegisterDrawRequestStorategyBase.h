#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStorategyBase : public StrategyBase<StaticModelComponent>
	{
	public:

		 StaticModelRegisterDrawRequestStorategyBase()          = default;
		~StaticModelRegisterDrawRequestStorategyBase() override = default;

		FWK_DEFINE_TYPE_INFO_ROOT(StaticModelRegisterDrawRequestStorategyBase)
	};
}