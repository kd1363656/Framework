#include "GameObjectHierarchyJsonConverter.h"

void FWK::Converter::GameObjectHierarchyJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy)
{
    if (a_rootJson.is_null()) { return; }

    // 削除済みUUID集合の復元のみ行う
    // 子UUIDListはSceneJsonConverter側で直接JSONから読み取って構築するため
    // ここでは復元しない
    if (const auto& l_json = a_rootJson.value(k_prefabRemovedChildUUIDSetJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializePrefabRemovedChildUUIDSet(l_json, a_gameObjectHierarchy);
    }
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::Serialize(const GameObjectHierarchy& a_gameObjectHierarchy) const
{
    nlohmann::json l_rootJson = {};

    // 子PrefabHierarchyNodeUUIDListのシリアライズ
    l_rootJson[k_childPrefabHierarchyNodeUUIDListJsonKey] = SerializeChildPrefabHierarchyNodeUUIDList(a_gameObjectHierarchy);

    // 削除済みUUID集合のシリアライズ
    l_rootJson[k_prefabRemovedChildUUIDSetJsonKey] = SerializePrefabRemovedChildUUIDSet(a_gameObjectHierarchy);

    return l_rootJson;
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializePrefabRemovedChildUUIDSet(const nlohmann::json& a_rootJson, GameObjectHierarchy& a_gameObjectHierarchy)
{
    if (!Utility::IsJsonArray(a_rootJson, k_prefabRemovedChildUUIDSetJsonKey)) { return; }
    
    for (const auto& l_json : a_rootJson[k_prefabRemovedChildUUIDSetJsonKey])
    {
        const auto& l_uuid = Utility::DeserializeUUID(l_json, k_uuidJsonKey);
 
        if (l_uuid.is_nil()) { continue; }
        
        // 削除されたコンポーネントをデシリアライズ
        a_gameObjectHierarchy.AddPrefabRemovedUUID(l_uuid);
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
        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_uuid, k_uuidJsonKey));

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
        Utility::UpdateJson(l_json, Utility::SerializeUUID(l_uuid, k_uuidJsonKey));
 
        l_childListJson.emplace_back(std::move(l_json));
    }
 
    return l_childListJson;
}