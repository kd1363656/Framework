#pragma once

namespace FWK
{
	class ComponentObserverInputExecutionCondition final : public IInputExecutionConditionBase
	{
	public:

		 ComponentObserverInputExecutionCondition() = default;
		~ComponentObserverInputExecutionCondition() = default;

		bool CanNotify(Observer<Enum::ComponentEvent>& a_componentEvent) override;

		void Deserialize(const nlohmann::json& a_rootJson) override;

		nlohmann::json Serialize() const override;

	private:

		std::vector<Struct::ComponentObserverInputExecutionConditionNode> m_conditionNodeList = {};

		FWK_DEFINE_TYPE_INFO(ComponentObserverInputExecutionCondition, IInputExecutionConditionBase)
	};
}