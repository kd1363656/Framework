#pragma once

namespace FWK
{
	class InputExecutionConditionBase final
	{
	public:

		 InputExecutionConditionBase() = default;
		~InputExecutionConditionBase() = default;

		virtual void CanNotify(Observer<Enum::ComponentEvent>& a_componentEvent) = 0;

		virtual void Deserialize(const nlohmann::json& a_rootJson) = 0;

		virtual nlohmann::json Serialize() const = 0;

		FWK_DEFINE_TYPE_INFO_ROOT(InputExecutionConditionBase)
	};
}