#include "GameObjectSceneJsonConverter.h"

bool FWK::Converter::GameObjectSceneJsonConverter::Deserialize(const nlohmann::json&                            a_rootJson, 
                                                                     std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet, 
                                                                     std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
                                                                     GameObject&                                a_gameObject, 
                                                                     Scene&                                     a_scene) const
{
    if (a_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "RootJsonが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

        return false;
    }

    // 保存していたUUIDを取得
    const auto l_sceneInstanceUUID = Utility::DeserializeUUID(a_rootJson, k_sceneInstanceUUIDJsonKey);

    if (l_sceneInstanceUUID.is_nil())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "SceneInstanceUUIDが無効のため、GameObjectのSceneデータをデシリアライズできませんでした。");

        return false;
    }

    // TransformComponentのシーンデータをデシリアライズ
    const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、GameObjectのSceneデータのデシリアライズに失敗しました。", false);

    l_transformComponent->DeserializeScene(a_rootJson.value(Constant::k_gameObjectTransformComponentJsonKey, nlohmann::json{}));

    // ComponentList + RemovedComponentUUIDListはContainerへ委譲
    auto& l_componentContainer = a_gameObject.GetMutableREFComponentContainer();

    if (!l_componentContainer.DeserializeScene(a_rootJson)) { return false; }

    // ComponentEventObserverのシーン側処理
    if (!DeserializeSceneComponentEventObserver(a_rootJson, a_gameObject)) { return false; }

    // ChildList + RemovedChildNodeUUIDListはHierarchyへ委譲
    if (!a_gameObject.GetMutableREFHierarchy().DeserializeScene(a_rootJson,
                                                                a_prefabUUIDSet,
                                                                a_childDeserializeDataList,
                                                                a_scene))
    {
        return false;
    }

    // シーンに配置されているゲームオブジェクトのUUID
    // 他のゲームオブジェクトがこのゲームオブジェクトに瞬時にアクセスしたいときなどに使用する
    a_gameObject.SetSceneInstanceUUID(l_sceneInstanceUUID);

    // PrefabDeserialize時点ではPrefabNameがSceneInstanceNameへ入っている。
    // Scene側に明示的な名前が保存されている場合は、
    // Scene固有情報としてここで上書きする
    if (const auto& l_sceneInstanceName = a_rootJson.value(k_nameJsonKey, std::string{});
        !l_sceneInstanceName.empty())
    {
        a_gameObject.SetName(l_sceneInstanceName);
    }

    return true;
}

nlohmann::json FWK::Converter::GameObjectSceneJsonConverter::Serialize(const GameObject& a_gameObject, const Scene& a_scene) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_sceneInstanceUUID = a_gameObject.GetREFSceneInstanceUUID();

    if (l_sceneInstanceUUID.is_nil()) { return {}; }

    l_rootJson[k_nameJsonKey] = a_gameObject.GetREFName();

    // PrefabUUIDはPrefabインスタンスの場合のみ書き込む
    if (const auto& l_prefabUUID = a_gameObject.GetREFPrefabUUID();
        !l_prefabUUID.is_nil())
    {
        Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_prefabUUID, Constant::k_gameObjectPrefabUUIDJsonKey));
    }

    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_sceneInstanceUUID, k_sceneInstanceUUIDJsonKey));

    // TransformComponentのシーンデータ
    const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのシーンのシリアライズに失敗しました。", {});

    if (const auto& l_json = l_transformComponent->SerializeScene();
        !l_json.is_null())
    {
        l_rootJson[Constant::k_gameObjectTransformComponentJsonKey] = l_json;
    }

    // ComponentList + RemovedComponentUUIDList
    const auto& l_componentContainer = a_gameObject.GetREFComponentContainer();

    Utility::UpdateJson(l_rootJson, l_componentContainer.SerializeScene());

    // Observer：存在すれば保存、Prefab由来が削除されたなら削除フラグを保存
    if (const auto& l_observer = a_gameObject.GetVALComponentEventObserver().lock())
    {
        l_rootJson[Constant::k_gameObjectComponentEventObserverJsonKey] = l_observer->Serialize();
    }
    else if (a_gameObject.GetVALIsPrefabObserverOrigin())
    {
        l_rootJson[Constant::k_gameObjectComponentEventObserverRemovedJsonKey] = true;
    }

    // ChildList + RemovedChildNodeUUIDList
    const auto& l_hierarchy = a_gameObject.GetREFHierarchy();

    Utility::UpdateJson(l_rootJson, l_hierarchy.SerializeScene());

    return l_rootJson;
}

bool FWK::Converter::GameObjectSceneJsonConverter::DeserializeSceneComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
    // Scene側でObserverを削除している場合は無効化する
    if (a_rootJson.value(Constant::k_gameObjectComponentEventObserverRemovedJsonKey, false))
    {
        a_gameObject.SetComponentEventObserver(nullptr);

        return true;
    }

    const auto& l_json = a_rootJson.value(Constant::k_gameObjectComponentEventObserverJsonKey, nlohmann::json{});

    if (l_json.is_null()) { return true; }

    // 既にPrefab由来のObserverがある場合はシーンの追記分を読み込む
    // ない場合はシーン側で追加されたObserverとして生成する
    if (const auto& l_observer = a_gameObject.GetVALComponentEventObserver().lock())
    {
        l_observer->Deserialize(l_json);

        return true;
    }

    auto l_observer = std::make_shared<Observer<Enum::ComponentEvent>>();

    l_observer->INIT        ();
    l_observer->Deserialize (l_json);

    a_gameObject.SetComponentEventObserver(l_observer);

    return true;
}