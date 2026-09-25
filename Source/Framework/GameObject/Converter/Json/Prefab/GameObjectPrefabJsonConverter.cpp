#include "GameObjectPrefabJsonConverter.h"

bool FWK::Converter::GameObjectPrefabJsonConverter::Deserialize(const std::weak_ptr<GameObject>&                 a_gameObject,
                                                                const nlohmann::json&                            a_rootJson,
                                                                      std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
                                                                      std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet,
                                                                      Scene&                                     a_scene) const
{
    if (a_rootJson.is_null()) { return false; }

    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return false; }

    const auto&           l_prefabUUID     = Utility::DeserializeUUID(a_rootJson, Constant::k_gameObjectPrefabUUIDJsonKey);
    const nlohmann::json* l_prefabBodyJson = &a_rootJson;

    if (!l_prefabUUID.is_nil())
    {
        // 既に同じPrefabUUIDがあるなら親と子に自分自身が存在するため
        // デシリアライズ処理を切り上げる
        if (!a_prefabUUIDSet.emplace(l_prefabUUID).second)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "プレハブのUUIDが重複しています、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

            return false;
        }

        // Prefabを識別するためのUUIDを格納
        l_gameObject->SetPrefabUUID(l_prefabUUID);

        auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem  ();
        auto* l_prefab       = l_prefabSystem.FindMutablePTRPrefab(l_prefabUUID);
 
        // プレハブが空なら登録したPrefabUUIDを削除
        if (!l_prefab)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "プレハブが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

            a_prefabUUIDSet.erase(l_prefabUUID);

            return false;
        }

        // プレハブからJSONファイルを取得
        l_prefabBodyJson = &l_prefab->GetREFJson();

        // しっかりJSONデータが読みとれていなければPrefabUUIDを解除
        if (l_prefabBodyJson->is_null())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "プレハブが保持しているJsonが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

            a_prefabUUIDSet.erase(l_prefabUUID);

            return false;
        }

        // 名前としてプレハブ名を格納しておく
        l_gameObject->SetName(l_prefab->GetREFPrefabName());
    }

    // 親Prefab内での子スロット識別UUIDを復元する
    // Prefabの子として生成された場合にのみ存在する
    if (const auto& l_prefabNodeUUID = Utility::DeserializeUUID(a_rootJson, Constant::k_gameObjectPrefabNodeUUIDJsonKey);
        !l_prefabNodeUUID.is_nil())
    {
        auto& l_hierarchy = l_gameObject->GetMutableREFHierarchy();

        l_hierarchy.SetPrefabNodeUUID(l_prefabNodeUUID);
    }

    // TransformComponentのプレハブデータを読み込む
    const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。", false);

    const auto& l_transformComponentJson = l_prefabBodyJson->value(Constant::k_gameObjectTransformComponentJsonKey, nlohmann::json{});

    l_transformComponent->DeserializePrefab(l_transformComponentJson);

    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();

    // ComponentListはContainerへ直接デシリアライズ
    if (!l_componentContainer.DeserializePrefab(*l_prefabBodyJson))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

        a_prefabUUIDSet.erase(l_prefabUUID); 
        
        return false;
    }

    // コンポーネント用オブザーバーのデシリアライズ
    if (const auto& l_json = l_prefabBodyJson->value(Constant::k_gameObjectComponentEventObserverJsonKey, nlohmann::json{});
        !l_json.is_null() &&
        !DeserializePrefabComponentEventObserver(l_json, *l_gameObject))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントオブザーバーのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

        a_prefabUUIDSet.erase(l_prefabUUID);
    
        return false;
    }

    // Prefab由来のObserverであることを記録する
    // Scene側でObserver削除の保存判定に使用する
    if (l_gameObject->GetVALComponentEventObserver().lock())
    {
        l_gameObject->SetIsPrefabObserverOrigin(true);
    }
     
    // ChildListはHierarchyへ委譲
    if (!l_gameObject->GetMutableREFHierarchy().DeserializePrefab(*l_prefabBodyJson,
                                                                  a_prefabUUIDSet,
                                                                  a_childDeserializeDataList,
                                                                  a_scene))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "子ゲームオブジェクトのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");
     
        a_prefabUUIDSet.erase(l_prefabUUID);

        return false;
    }
     
    // このPrefab以下のデシリアライズがすべて終了したため、
    // 兄弟Prefabで同じUUIDを使用できるように現在経路から削除する
    a_prefabUUIDSet.erase(l_prefabUUID);

    return true;
}

nlohmann::json FWK::Converter::GameObjectPrefabJsonConverter::Serialize(const GameObject& a_gameObject) const
{
          nlohmann::json l_rootJson   = {};
    const auto&          l_prefabUUID = a_gameObject.GetREFPrefabUUID();
    
    // Prefab化されていないGameObject、もしくわシーンインスタンス名が
    // 割り当てられていないゲームオブジェクトは、
    // Sceneの保存対象にしない
    if (!l_prefabUUID.is_nil())
    {
        // PrefabUUIDを保存
        Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_prefabUUID, Constant::k_gameObjectPrefabUUIDJsonKey));
    }

    // TransformComponent（既存処理を残す）
    const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();
     
    FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブのシリアライズに失敗しました。", {});
     
    nlohmann::json l_transformJson = {};
    
    Utility::UpdateJson(l_transformJson, l_transformComponent->SerializePrefab());
     
    l_rootJson[Constant::k_gameObjectTransformComponentJsonKey] = l_transformJson;
     
    // ComponentListはContainerへ委譲
    Utility::UpdateJson(l_rootJson, a_gameObject.GetREFComponentContainer().SerializePrefab());
     
    // コンポーネントオブザーバのシリアライズ（既存ブロック、キーはConstantへ）
    if (const auto& l_json = SerializePrefabComponentObserver(a_gameObject);
        !l_json.is_null())
    {
        l_rootJson[Constant::k_gameObjectComponentEventObserverJsonKey] = l_json;
    }
     
    // ChildListはHierarchyへ委譲
    Utility::UpdateJson(l_rootJson, a_gameObject.GetREFHierarchy().SerializePrefab());
     
    return l_rootJson;
}

bool FWK::Converter::GameObjectPrefabJsonConverter::DeserializePrefabComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
    if (a_rootJson.is_null()) { return false; }

    // ComponentEventObserverのJsonがnull出ない場合std::make_uniqueでインスタンス化
    auto l_componentEventObserver = std::make_shared<Observer<Enum::ComponentEvent>>();

    l_componentEventObserver->INIT();

    l_componentEventObserver->Deserialize(a_rootJson);

    a_gameObject.SetComponentEventObserver(l_componentEventObserver);

    return true;
}

nlohmann::json FWK::Converter::GameObjectPrefabJsonConverter::SerializePrefabComponentObserver(const GameObject& a_gameObject) const
{
    const auto& l_componentEventObserver = a_gameObject.GetVALComponentEventObserver().lock();

    if (!l_componentEventObserver) { return {}; }

    return l_componentEventObserver->Serialize();
}