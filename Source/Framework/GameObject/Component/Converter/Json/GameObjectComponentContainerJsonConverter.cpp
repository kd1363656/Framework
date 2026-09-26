#include "GameObjectComponentContainerJsonConverter.h"

void FWK::Converter::GameObjectComponentContainerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer)
{
    if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::Serialize(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    nlohmann::json l_rootJson = {};
 
    return l_rootJson;
}