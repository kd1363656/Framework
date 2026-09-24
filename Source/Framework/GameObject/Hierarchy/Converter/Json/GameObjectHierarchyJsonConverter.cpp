#include "GameObjectHierarchyJsonConverter.h"

bool FWK::Converter::GameObjectHierarchyJsonConverter::DeserializePrefab(const nlohmann::json&                            a_rootJson, 
                                                                               std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet, 
                                                                               std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
                                                                               Scene&                                     a_scene) const
{
    return false;
}
bool FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeScene(const nlohmann::json&                            a_rootJson, 
                                                                              std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                                                                              std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
                                                                              GameObjectHierarchy&                       a_gameObjectHierarchy,
                                                                              Scene&                                     a_scene) const
{
    return false;
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializePrefab(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    return nlohmann::json();
}
nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeScene(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    return nlohmann::json();
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeRemovedUUIDList(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    return nlohmann::json();
}