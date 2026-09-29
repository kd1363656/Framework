#include "GameObjectHierarchyJsonConverter.h"

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeScene(const nlohmann::json&              a_rootJson,
                                                                        const nlohmann::json&              a_prefabJson, 
                                                                        const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                              GameObjectHierarchy&         a_gameObjectHierarchy, 
                                                                              Scene&                       a_scene) const
{
    // シーン側がnullなら何も読み込まない
    if (a_rootJson.is_null()) { return; }

    const auto& l_sceneChildListJson = a_rootJson.value(k_childListJsonKey, nlohmann::json{});
 
    // Prefab基底が無ければシーン独自GameObjectとしてフル形式で読み込む
    if (a_prefabJson.is_null())
    {
        DeserializeChildList(l_sceneChildListJson, 
                             a_prefabSystem, 
                             a_gameObjectHierarchy,
                             a_scene);
 
        return;
    }
 
    // Prefab基底のChildListが配列として存在し、
    // かつシーン側が差分構造(配列以外/Null/キーなし)ならPrefab + 差分マージして読む
    if (Utility::IsJsonArray(a_prefabJson, k_childListJsonKey) &&
        !l_sceneChildListJson.is_array())
    {
        // 削除済みUUIDをHierarchyの削除UUIDセットへ登録
        // AddChildが削除UUIDとの重複をはじくための情報になる
        DeserializeRemovedUUIDList(l_sceneChildListJson, a_gameObjectHierarchy);
 
        const auto& l_childListJson = a_prefabJson.value(k_childListJsonKey, nlohmann::json{});

        // Prefab基底の子ノードと差分を照合しながら子を再構築する
        DeserializeChildListDiff(l_childListJson,
                                 l_sceneChildListJson,
                                 a_prefabSystem,
                                 a_gameObjectHierarchy,
                                 a_scene);
 
        return;
    }
 
    // シーン側が配列形式ならフル形式として読み込む
    DeserializeChildList(l_sceneChildListJson, 
                         a_prefabSystem,
                         a_gameObjectHierarchy,
                         a_scene);
}
void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializePrefab(const nlohmann::json&              a_rootJson, 
                                                                         const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                               GameObjectHierarchy&         a_gameObjectHierarchy, 
                                                                               Scene&                       a_scene) const
{
    if (a_rootJson.is_null()) { return; }
 
    // PrefabファイルのChildListは常にフル形式(配列)で保存される
    DeserializeChildList(a_rootJson.value(k_childListJsonKey, nlohmann::json{}),
                         a_prefabSystem,
                         a_gameObjectHierarchy,
                         a_scene);
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::Serialize(const GameObjectHierarchy& a_gameObjectHierarchy, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    nlohmann::json l_rootJson = {};

    auto l_jsonArray = nlohmann::json::array();

    const auto& l_childSmartPointerVectorList = a_gameObjectHierarchy.GetREFChildSmartPointerVectorList();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList    ();

    for (const auto& l_childData : l_childDataList)
    {
        // 子リストはweak_ptrで保持されているためlockで取得
        const auto& l_child = l_childData.m_type.lock();

        if (!l_child) { continue; }

        // 子はシーン形式エントリとしてシリアライズ
        // 非Prefabならフル形式、別Prefabインスタンスなら差分形式になるのは子側で判定する
        l_jsonArray.emplace_back(l_child->SerializeScene(a_prefabSystem));
    }

    l_rootJson[k_childListJsonKey] = std::move(l_jsonArray);

    return l_rootJson;
}
nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeDiff(const nlohmann::json& a_prefabJson, const GameObjectHierarchy& a_gameObjectHierarchy, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    const auto& l_prefabChildListJson = a_prefabJson.value(k_childListJsonKey, nlohmann::json{});

    // Prefab基底が無ければ差分は作れないので空の差分構造を返す
    if (l_prefabChildListJson.is_null() ||
        !Utility::IsJsonArray(l_prefabChildListJson))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefab側ChildListが無効なため、差分シリアライズに失敗しました。");

        nlohmann::json l_emptyDiffJson = {};

        l_emptyDiffJson[k_addedJsonKey]           = nlohmann::json::array();
        l_emptyDiffJson[k_removedUUIDListJsonKey] = nlohmann::json::array();
        l_emptyDiffJson[k_modifiedJsonKey]        = nlohmann::json::array();

        return l_emptyDiffJson;
    }

    auto l_addedJsonArray    = nlohmann::json::array();
    auto l_modifiedJsonArray = nlohmann::json::array();
 
    // RemovedはUUIDのsetで集計してからシリアライズする
    std::unordered_set<boost::uuids::uuid> l_removedUUIDSet = {};
 
    // PrefabノードをPrefabHierarchyNodeUUIDで引けるようにする
    std::unordered_map<boost::uuids::uuid, const nlohmann::json*> l_prefabNodeMap  = {};
    std::unordered_set<boost::uuids::uuid>                        l_matchedUUIDSet = {};

    // NodeUUIDとそれに対応するJsonデータを格納
    for (const auto& l_prefabNodeJson : l_prefabChildListJson)
    {
        const auto& l_nodeUUID = Utility::DeserializeUUID(l_prefabNodeJson, k_prefabHierarchyNodeUUIDJsonKey);
 
        if (l_nodeUUID.is_nil()) { continue; }
 
        l_prefabNodeMap.try_emplace(l_nodeUUID, &l_prefabNodeJson);
    }

    const auto& l_childSmartPointerVectorList = a_gameObjectHierarchy.GetREFChildSmartPointerVectorList();
    const auto& l_childDataList               = l_childSmartPointerVectorList.GetREFElementDataList    ();

    // 現在の子を走査してAddedとModifiedに振り分ける
    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();
 
        if (!l_child) { continue; }
 
        const auto& l_nodeUUID = l_child->GetREFPrefabHierarchyNodeUUID();
        const auto& l_itr      = l_prefabNodeMap.find                  (l_nodeUUID);
 
        // Prefabノードに対応しない = インスタンス独自の追加子
        // ※別Prefabインスタンスの場合もここに入る(差分形式かどうかは子側で判定)
        if (l_itr == l_prefabNodeMap.end())
        {
            l_addedJsonArray.emplace_back(l_child->SerializeScene(a_prefabSystem));
 
            continue;
        }
 
        l_matchedUUIDSet.emplace(l_nodeUUID);
 
        // Prefabノードとの差分を子自身にシリアライズさせる
        // SceneInstanceUUID等のメタ情報も含まれるため、変更ゼロでもエントリは必ず出す
        nlohmann::json l_modifiedJson = {};
 
        Utility::UpdateJson(l_modifiedJson, Utility::SerializeUUID(l_nodeUUID, k_prefabHierarchyNodeUUIDJsonKey));
 
        l_modifiedJson[k_gameObjectDataJsonKey] = l_child->SerializeDiff(*l_itr->second);
 
        l_modifiedJsonArray.emplace_back(std::move(l_modifiedJson));
    }

    // Prefabにあって現在に無いノード = インスタンスで削除済み
    for (const auto& [l_nodeUUID, l_prefabNodeJson] : l_prefabNodeMap)
    {
        if (l_matchedUUIDSet.contains(l_nodeUUID)) { continue; }
 
        l_removedUUIDSet.emplace(l_nodeUUID);
    }
 
    nlohmann::json l_diffJson = {};
 
    l_diffJson[k_addedJsonKey]           = l_addedJsonArray;
    l_diffJson[k_removedUUIDListJsonKey] = SerializeRemovedUUIDList(l_removedUUIDSet);
    l_diffJson[k_modifiedJsonKey]        = l_modifiedJsonArray;
 
    return l_diffJson;
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChildList(const nlohmann::json&              a_childListJson, 
                                                                            const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                                  GameObjectHierarchy&         a_gameObjectHierarchy,
                                                                                  Scene&                       a_scene) const
{

}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChildListDiff(const nlohmann::json&              a_prefabChildListJson, 
                                                                                const nlohmann::json&              a_childListDiffJson, 
                                                                                const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                                      GameObjectHierarchy&         a_gameObjectHierarchy, 
                                                                                      Scene&                       a_scene) const
{

}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChild(const std::weak_ptr<GameObject>&   a_parentGameObject, 
                                                                        const nlohmann::json&              a_childJson, 
                                                                        const nlohmann::json&              a_baseJson, 
                                                                        const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                              Scene&                       a_scene) const
{

}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeRemovedUUIDList(const nlohmann::json& a_childListDiffJson, GameObjectHierarchy& a_gameObjectHierarchy) const
{

}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeRemovedUUIDList(const std::unordered_set<boost::uuids::uuid>&a_removedUUIDSet) const
{
    return nlohmann::json();
}