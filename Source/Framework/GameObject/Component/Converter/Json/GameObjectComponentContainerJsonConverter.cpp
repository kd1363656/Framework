#include "GameObjectComponentContainerJsonConverter.h"

bool FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return false;
}
bool FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return false;
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializePrefab(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return nlohmann::json();
}
nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeScene(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return nlohmann::json();
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeRemovedUUIDList(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return nlohmann::json();
}