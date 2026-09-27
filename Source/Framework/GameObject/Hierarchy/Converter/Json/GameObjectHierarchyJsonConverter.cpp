#include "GameObjectHierarchyJsonConverter.h"

void FWK::Converter::GameObjectHierarchyJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy)
{
    if (a_rootJson.is_null()) { return; }
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::Serialize(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    nlohmann::json l_rootJson = {};

    return l_rootJson;
}