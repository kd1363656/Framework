#include "GameObjectJsonConverter.h"

void FWK::Converter::GameObjectJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, const std::weak_ptr<GameObject>& a_gameObject) const
{
    if (a_rootJson.is_null()) { return; }
 
    const auto& l_gameObject = a_gameObject.lock();
 
    if (!l_gameObject) { return; }
 
    // 共通するデシリアライズ処理を実行
    DeserializeCommon(a_rootJson, *l_gameObject);

    // ComponentContainerのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_componentContainerJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();
 
        l_componentContainer.Deserialize(l_json);
    }
 
    // Hierarchyのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_hierarchyJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();
 
        l_hierarchy.DeserializeScene(l_json);
    }
}
void FWK::Converter::GameObjectJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, const std::weak_ptr<GameObject>& a_gameObject) const
{
    if (a_rootJson.is_null()) { return; }
 
    const auto& l_gameObject = a_gameObject.lock();
 
    if (!l_gameObject) { return; }
 
    // 共通するデシリアライズ処理を実行
    DeserializeCommon(a_rootJson, *l_gameObject);
 
    // ComponentContainer
    if (const auto& l_json = a_rootJson.value(k_componentContainerJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();
 
        l_componentContainer.Deserialize(l_json);
    }
 
    // Hierarchy
    // Prefab用DeserializeではChildGameObjectListから子GameObjectも生成する
    if (const auto& l_json = a_rootJson.value(k_hierarchyJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();
 
        l_hierarchy.DeserializePrefab(l_json);
    }
}

nlohmann::json FWK::Converter::GameObjectJsonConverter::Serialize(const GameObject& a_gameObject) const
{
    nlohmann::json l_rootJson = {};

    // Nameのシリアライズ
    l_rootJson[k_nameJsonKey] = a_gameObject.GetREFName();
 
    // PrefabUUIDのシリアライズ
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabUUID(), k_prefabUUIDJsonKey));
 
    // PrefabHierarchyNodeUUIDのシリアライズ
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabHierarchyNodeUUID(), k_prefabHierarchyNodeUUIDJsonKey));
 
    // SceneInstanceUUIDのシリアライズ
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFSceneInstanceUUID(), k_sceneInstanceUUIDJsonKey));
 
    // IsPrefabOriginのシリアライズ
    l_rootJson[k_isPrefabOriginJsonKey] = a_gameObject.GetVALIsPrefabOrigin();
 
    // Transformのシリアライズ
    if (const auto& l_transform = a_gameObject.GetVALTransformComponent().lock())
    {
        l_rootJson[k_transformJsonKey] = l_transform->Serialize();
    }
    else
    {
        FWK_ASSERT_RETURN_VALUE("TransformComponentがインスタンス化されておらず、ゲームオブジェクトのシリアライズ処理に失敗しました。", nlohmann::json{});
    }
 
    const auto& l_componentContainer = a_gameObject.GetREFComponentContainer();
    const auto& l_hierarchy          = a_gameObject.GetREFHierarchy         ();

    // ComponentContainerのシリアライズ
    l_rootJson[k_componentContainerJsonKey] = l_componentContainer.Serialize();
 
    // Hierarchyのシリアライズ
    l_rootJson[k_hierarchyJsonKey] = l_hierarchy.Serialize();
 
    return l_rootJson;
}

void FWK::Converter::GameObjectJsonConverter::DeserializeCommon(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
    // TransformComponentのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_transformJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        if (const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock())
        {
            l_transformComponent->Deserialize(l_json);
        }
        else
        {
            FWK_ASSERT_RETURN("TransformComponentがインスタンス化されておらず、ゲームオブジェクトのデシリアライズ処理に失敗しました。");
        }
    }

    // Nameのデシリアライズ
    const auto& l_name = a_rootJson.value(k_nameJsonKey, std::string{});

    a_gameObject.SetName(l_name);
 
    // PrefabUUIDのデシリアライズ
    if (const auto& l_uuid = Utility::DeserializeUUID(a_rootJson, k_prefabUUIDJsonKey);
        !l_uuid.is_nil())
    {
        a_gameObject.SetPrefabUUID(l_uuid);
    }

    // PrefabHierarchyNodeUUIDのデシリアライズ
    if (const auto& l_uuid = Utility::DeserializeUUID(a_rootJson, k_prefabHierarchyNodeUUIDJsonKey);
        !l_uuid.is_nil())
    {
        a_gameObject.SetPrefabHierarchyNodeUUID(l_uuid);
    }
 
    // SceneInstanceUUIDのデシリアライズ
    if (const auto& l_uuid = Utility::DeserializeUUID(a_rootJson, k_sceneInstanceUUIDJsonKey);
        !l_uuid.is_nil())
    {
        a_gameObject.SetSceneInstanceUUID(l_uuid);
    }
 
    // プレハブかどうかのデシリアライズ
    const bool l_isPrefabOrigin = a_rootJson.value(k_isPrefabOriginJsonKey, Constant::l_gameObjectInitialValueIsPrefabOriginValue);

    a_gameObject.SetIsPrefabOrigin(l_isPrefabOrigin);
 
}