#pragma once

namespace FWK
{
	class InputComponent final : public ComponentBase
	{
	public:

		 InputComponent()          = default;
		~InputComponent() override = default;

		void INIT() override;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		void EarlyUpdate() override;

		nlohmann::json SerializePrefab() override;

		bool IsAllowMultiple() const override { return true; }

		void EditInspector() override;

		bool CanNotifyInputEvent(Observer<Enum::ComponentEvent>& a_componentEventObserver);

		void AddExecutionConditionList(const Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition);

		const auto& GetREFNotifyComponentEventExecutionConditionList() const { return m_notifyComponentEventExecutionConditionList; }

		const auto& GetREFInspector() const { return m_inspector; }

		auto& GetMutableREFNotifyComponentEventExecutionConditionList() { return m_notifyComponentEventExecutionConditionList; }

		auto& GetMutableREFInspector() { return m_inspector; }

	private:

		std::vector<Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>> m_notifyComponentEventExecutionConditionList = {};

		InputComponentInspector m_inspector = {};

		Struct::InputComponentExecution m_execution = {};

		Converter::InputComponentJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(InputComponent, ComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::InputComponent)