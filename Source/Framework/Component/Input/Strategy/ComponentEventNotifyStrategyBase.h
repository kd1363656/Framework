#pragma once

namespace FWK
{
	class ComponentEventNotifyStrategyBase : public StrategyBase<FWK::InputComponent>
	{
	public:

		 ComponentEventNotifyStrategyBase()          = default;
		~ComponentEventNotifyStrategyBase() override = default;

		FWK_DEFINE_TYPE_INFO_ROOT(ComponentEventNotifyStrategyBase)
	};
}