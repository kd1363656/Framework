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

		FWK_DEFINE_TYPE_INFO(ComponentObserverInputExecutionCondition, IInputExecutionConditionBase)
	};
}