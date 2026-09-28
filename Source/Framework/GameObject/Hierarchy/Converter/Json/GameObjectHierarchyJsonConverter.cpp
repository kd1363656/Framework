#include "GameObjectHierarchyJsonConverter.h"

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy)
{
    if (a_rootJson.is_null()) { return; }

    // PrefabRemovedChildUUIDSetのデシリアライズ
    if (const auto& l_json = a_rootJson.value(Constant::k_gameObjectHierarchyJsonConverterPrefabRemovedChildUUIDSetJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializePrefabRemovedChildUUIDSet(l_json, a_gameObjectHierarchy);
    }
}
void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy)
{
    if (a_rootJson.is_null()) { return; }
 
    // PrefabRemovedChildUUIDSetのデシリアライズ
    if (const auto& l_json = a_rootJson.value(Constant::k_gameObjectHierarchyJsonConverterPrefabRemovedChildUUIDSetJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializePrefabRemovedChildUUIDSet(l_json, a_gameObjectHierarchy);
    }
 
    // ChildGameObjectListのデシリアライズ
    if (const auto& l_json = a_rootJson.value(Constant::k_gameObjectHierarchyJsonConverterChildGameObjectListJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializeChildGameObjectList(l_json, a_gameObjectHierarchy);
    }
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::Serialize(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    nlohmann::json l_rootJson = {};

    // ChildPrefabHierarchyNodeUUIDListのシリアライズ
    l_rootJson[Constant::k_gameObjectHierarchyJsonConverterChildPrefabHierarchyNodeUUIDListJsonKey] = SerializeChildPrefabHierarchyNodeUUIDList(a_gameObjectHierarchy);

    // PrefabRemovedChildUUIDSetのシリアライズ
    l_rootJson[Constant::k_gameObjectHierarchyJsonConverterPrefabRemovedChildUUIDSetJsonKey] = SerializePrefabRemovedChildUUIDSet(a_gameObjectHierarchy);

    // ChildGameObjectListのシリアライズ
    l_rootJson[Constant::k_gameObjectHierarchyJsonConverterChildGameObjectListJsonKey] = SerializeChildGameObjectList(a_gameObjectHierarchy);
 
    return l_rootJson;
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializePrefabRemovedChildUUIDSet(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson)) 
    {
        return; 
    }
    
    // 配列JSONを直接ループする
    for (const auto& l_json : a_rootJson)
    {
        const auto& l_uuid = Utility::DeserializeUUID(l_json, Constant::k_gameObjectHierarchyJsonConverterUUIDJsonKey);

        if (l_uuid.is_nil()) { continue; }

        a_gameObjectHierarchy.AddPrefabRemovedUUID(l_uuid);
    }
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChildGameObjectList(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy) const
{
    if (a_rootJson.is_null() || 
        !Utility::IsJsonArray(a_rootJson)) 
    {
        return; 
    }
 
    const auto& l_owner = a_gameObjectHierarchy.GetREFOwner().lock();
 
    if (!l_owner) { return; }
 
    for (const auto& l_json : a_rootJson)
    {
        if (l_json.is_null()) { continue; }
 
        auto l_child = std::make_shared<GameObject>();
 
        l_child->INIT();
 
        // Prefab用Deserializeで子GameObjectを生成する
        // これで子GameObjectもPrefab UUIDを持つようになる
        l_child->DeserializePrefab(l_json);
 
        // ApplyParentを使わずに直接親子リンクを設定する
        auto& l_childHierarchy = l_child->GetMutableREFHierarchy();

        l_childHierarchy.ConnectParentForDeserialize(l_owner);
    }
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializePrefabRemovedChildUUIDSet(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    nlohmann::json l_removedSetJson = nlohmann::json::array();

    const auto& l_prefabRemovedChildUUIDSet = a_gameObjectHierarchy.GetREFPrefabRemovedChildUUIDSet();

    for (const auto& l_uuid : l_prefabRemovedChildUUIDSet)
    {
        if (l_uuid.is_nil()) { continue; }

        nlohmann::json l_json = {};

        // 削除された子ゲームオブジェクトのUUIDを保存
        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_uuid, Constant::k_gameObjectHierarchyJsonConverterUUIDJsonKey));

        l_removedSetJson.emplace_back(std::move(l_json));
    }

    return l_removedSetJson;
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeChildPrefabHierarchyNodeUUIDList(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    nlohmann::json l_childListJson = nlohmann::json::array();
 
    const auto& l_childSmartPointerVectorList = a_gameObjectHierarchy.GetREFChildSmartPointerVectorList();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList    ();
 
    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child) { continue; }
 
        const auto& l_uuid = l_child->GetREFPrefabHierarchyNodeUUID();
 
        if (l_uuid.is_nil()) { continue; }
 
        nlohmann::json l_json = {};
 
        // 子GameObjectとしてのUUIDを保存
        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_uuid, Constant::k_gameObjectHierarchyJsonConverterUUIDJsonKey));
 
        l_childListJson.emplace_back(std::move(l_json));
    }
 
    return l_childListJson;
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeChildGameObjectList(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    nlohmann::json l_childGameObjectListJson = nlohmann::json::array();
 
    const auto& l_childSmartPointerVectorList = a_gameObjectHierarchy.GetREFChildSmartPointerVectorList();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList    ();
 
    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child) { continue; }
 
        l_childGameObjectListJson.emplace_back(l_child->Serialize());
    }
 
    return l_childGameObjectListJson;
}