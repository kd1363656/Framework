#pragma once

namespace FWK
{
	class IInputExecutionConditionBase
	{
	public:

		 IInputExecutionConditionBase() = default;
		~IInputExecutionConditionBase() = default;

		virtual bool CanNotify(Observer<Enum::ComponentEvent>& a_componentEvent) = 0;

		virtual void Deserialize(const nlohmann::json& a_rootJson) = 0;

		virtual nlohmann::json Serialize() const = 0;

		FWK_DEFINE_TYPE_INFO_ROOT(IInputExecutionConditionBase)
	};
}