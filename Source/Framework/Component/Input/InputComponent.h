#pragma once

namespace FWK
{
	class ComponentEventNotifyStrategyBase;
}

namespace FWK
{
	class InputComponent final : public ComponentBase
	{
	public:

		 InputComponent()          = default;
		~InputComponent() override = default;

		void INIT() override;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		void PostDeserialize() override;

		void EarlyUpdate() override;

		nlohmann::json SerializePrefab() override;

		bool IsAllowMultiple() const override { return true; }

		void EditInspector() override;

		bool CanNotifyEvent(Observer<Enum::ComponentEvent>& a_componentEventObserver);

		void AddExecutionConditionList(const Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition);

		void SetNotifyStrategy(std::unique_ptr<ComponentEventNotifyStrategyBase>&& a_set) { m_notifyStrategy = std::move(a_set); }

		const auto& GetREFNotifyComponentEventExecutionConditionList() const { return m_notifyComponentEventExecutionConditionList; }

		const auto& GetREFNotifyStrategy() const { return m_notifyStrategy; }

		const auto& GetREFInspector() const { return m_inspector; }
		const auto& GetREFExecution() const { return m_execution; }

		auto& GetMutableREFNotifyComponentEventExecutionConditionList() { return m_notifyComponentEventExecutionConditionList; }

		auto& GetMutableREFNotifyStrategy() { return m_notifyStrategy; }

		auto& GetMutableREFInspector() { return m_inspector; }
		auto& GetMutableREFExecution() { return m_execution; }

	private:

		void NotifyEvent();

		std::vector<Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>> m_notifyComponentEventExecutionConditionList = {};

		std::unique_ptr<ComponentEventNotifyStrategyBase> m_notifyStrategy = nullptr;

		InputComponentInspector m_inspector = {};

		Struct::InputComponentExecution m_execution = {};

		Converter::InputComponentJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(InputComponent, ComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::InputComponent)