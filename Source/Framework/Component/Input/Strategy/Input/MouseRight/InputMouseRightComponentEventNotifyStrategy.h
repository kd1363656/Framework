#pragma once

namespace FWK
{
	class InputMouseRightComponentEventNotifyStrategy : public ComponentEventNotifyStrategyBase
	{
	public:

		 InputMouseRightComponentEventNotifyStrategy()          = default;
		~InputMouseRightComponentEventNotifyStrategy() override = default;

		void Execute(InputComponent& a_inputComponent) override;

		FWK_DEFINE_TYPE_INFO(InputMouseRightComponentEventNotifyStrategy, ComponentEventNotifyStrategyBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentEventNotifyStrategyUniqueFactory, FWK::InputMouseRightComponentEventNotifyStrategy)