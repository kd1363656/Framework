#include "ComponentObserverInputExecutionCondition.h"

bool FWK::ComponentObserverInputExecutionCondition::CanNotify(Observer<Enum::ComponentEvent>& a_componentEvent)
{
	return true;
}

void FWK::ComponentObserverInputExecutionCondition::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::ComponentObserverInputExecutionCondition::Serialize() const
{
	return nlohmann::json();
}