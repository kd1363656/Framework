#include "GameObjectHierarchyJsonConverter.h"

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeScene(const nlohmann::json&              a_rootJson,
                                                                        const nlohmann::json&              a_prefabJson, 
                                                                        const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                              GameObjectHierarchy&         a_gameObjectHierarchy, 
                                                                              Scene&                       a_scene) const
{
    // シーン側がnullなら何も読み込まない
    if (a_rootJson.is_null()) { return; }

    const auto& l_sceneChildListJson = a_rootJson.value(Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey, nlohmann::json{});
 
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
    if (Utility::IsJsonArray(a_prefabJson, Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey) &&
        !l_sceneChildListJson.is_array())
    {
        // 削除済みUUIDをHierarchyの削除UUIDセットへ登録
        // AddChildが削除UUIDとの重複をはじくための情報になる
        DeserializeRemovedUUIDList(l_sceneChildListJson, a_gameObjectHierarchy);
 
        const auto& l_childListJson = a_prefabJson.value(Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey, nlohmann::json{});

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
 
    const auto& l_childListJsonArray = a_rootJson.value(Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey, nlohmann::json{});

    // PrefabファイルのChildListは常にフル形式(配列)で保存される
    DeserializeChildList(l_childListJsonArray,
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

    l_rootJson[Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey] = std::move(l_jsonArray);

    return l_rootJson;
}
nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeDiff(const nlohmann::json& a_prefabJson, const GameObjectHierarchy& a_gameObjectHierarchy, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    const auto& l_rootJson = a_prefabJson.value(Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey, nlohmann::json{});

    // Prefab基底が無ければ差分は作れないので空の差分構造を返す
    if (l_rootJson.is_null() ||
        !Utility::IsJsonArray(l_rootJson))
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
    for (const auto& l_json : l_rootJson)
    {
        const auto& l_nodeUUID = Utility::DeserializeUUID(l_json, k_prefabHierarchyNodeUUIDJsonKey);
 
        if (l_nodeUUID.is_nil()) { continue; }
 
        l_prefabNodeMap.try_emplace(l_nodeUUID, &l_json);
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

    // 順序差分の検出
    // 自然順 = Prefabの子順(Removed除く) + Added末尾
    std::vector<boost::uuids::uuid> l_naturalOrderUUIDList = {};
    std::vector<boost::uuids::uuid> l_currentOrderUUIDList = {};
     
    // Prefab側に存在する子GameObjectのNodeUUIDをデシリアライズ
    for (const auto& l_prefabNodeJson : l_rootJson)
    {
        const auto& l_nodeUUID = Utility::DeserializeUUID(l_prefabNodeJson, k_prefabHierarchyNodeUUIDJsonKey);
     
        if (l_nodeUUID.is_nil() ||
            l_removedUUIDSet.contains(l_nodeUUID))
        {
            continue;
        }
     
        l_naturalOrderUUIDList.emplace_back(l_nodeUUID);
    }
     
    // プレハブに存在しないUUIDがあったらNaturealOrderUUIDListに追加しておく
    for (const auto& l_childData : l_childDataList)
    {
        const auto& l_child = l_childData.m_type.lock();
     
        if (!l_child) { continue; }
     
        const auto& l_nodeUUID = l_child->GetREFPrefabHierarchyNodeUUID();
     
        if (l_nodeUUID.is_nil()) { continue; }
     
        // Prefabに無い = AddedのnodeUUIDでもある
        if (!l_prefabNodeMap.contains(l_nodeUUID))
        {
            l_naturalOrderUUIDList.emplace_back(l_nodeUUID);
        }
     
        l_currentOrderUUIDList.emplace_back(l_nodeUUID);
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
 
    // 自然順と現在順が異なる場合のみ順序を保存する
    if (l_naturalOrderUUIDList != l_currentOrderUUIDList)
    {
        auto l_orderJsonArray = nlohmann::json::array();
     
        for (const auto& l_nodeUUID : l_currentOrderUUIDList)
        {
            l_orderJsonArray.emplace_back(Utility::SerializeUUID(l_nodeUUID, k_prefabHierarchyNodeUUIDJsonKey));
        }
     
        l_diffJson[k_orderUUIDListJsonKey] = std::move(l_orderJsonArray);
    }



    return l_diffJson;
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChildList(const nlohmann::json&              a_childListJson, 
                                                                            const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                                  GameObjectHierarchy&         a_gameObjectHierarchy,
                                                                                  Scene&                       a_scene) const
{
    if (a_childListJson.is_null() ||
        !Utility::IsJsonArray(a_childListJson)) 
    {
        return; 
    }
 
    // このHierarchyのOwner = 子たちの親になるGameObject
    const auto& l_parent = a_gameObjectHierarchy.GetREFOwner();
 
    // 配列順に処理 = AddChild(ConnectParentForDeserialize経由)の呼び出し順になるので
    // JSONの並びがそのまま子リストの並びとして復元される
    for (const auto& l_json : a_childListJson)
    {
        // フル形式なのでPrefab基底は無し(NullJson)
        DeserializeChild(l_parent,
                         l_json,
                         nlohmann::json{},
                         a_prefabSystem,
                         a_scene);
    }
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChildListDiff(const nlohmann::json&              a_prefabChildListJson, 
                                                                                const nlohmann::json&              a_childListDiffJson, 
                                                                                const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                                      GameObjectHierarchy&         a_gameObjectHierarchy, 
                                                                                      Scene&                       a_scene) const
{
    if (a_prefabChildListJson.is_null() ||
        !Utility::IsJsonArray(a_prefabChildListJson))
    {
        return;
    }

    // このHierarchyのOwner = 子たちの親になるGameObject
    const auto& l_parent = a_gameObjectHierarchy.GetREFOwner();

    // 各PrefabHierarchyNodeUUIDからJsonを引けるようにする
    std::unordered_map<boost::uuids::uuid, const nlohmann::json*> l_prefabNodeMap   = {};
    std::unordered_map<boost::uuids::uuid, const nlohmann::json*> l_modifiedDataMap = {};
    std::unordered_map<boost::uuids::uuid, const nlohmann::json*> l_addedMap        = {};
 
    // Addedの配列順を保持するためのUUIDリスト
    std::vector<boost::uuids::uuid> l_addedUUIDList = {};

    // PrefabNodeUUIDで子のNodeUUIDとそれに対応するJsonを格納
    for (const auto& l_prefabNodeJson : a_prefabChildListJson)
    {
        const auto& l_nodeUUID = Utility::DeserializeUUID(l_prefabNodeJson, k_prefabHierarchyNodeUUIDJsonKey);

        if (l_nodeUUID.is_nil()) { continue; }

        l_prefabNodeMap.try_emplace(l_nodeUUID, &l_prefabNodeJson);
    }

    const auto& l_modifiedJsonArray = a_childListDiffJson.value(k_modifiedJsonKey, nlohmann::json{});
    const auto& l_addedJsonArray    = a_childListDiffJson.value(k_addedJsonKey,    nlohmann::json{});

    if (l_modifiedJsonArray.is_array())
    {
        for (const auto& l_modifiedJson : l_modifiedJsonArray)
        {
            const auto& l_nodeUUID = Utility::DeserializeUUID(l_modifiedJson, k_prefabHierarchyNodeUUIDJsonKey);
 
            if (l_nodeUUID.is_nil()) { continue; }
 
            l_modifiedDataMap.try_emplace(l_nodeUUID, &l_modifiedJson);
        }
    }

    if (l_addedJsonArray.is_array())
    {
        for (const auto& l_addedJson : l_addedJsonArray)
        {
            // Addedエントリ自身のPrefabHierarchyNodeUUIDを読む
            const auto& l_nodeUUID = Utility::DeserializeUUID(l_addedJson, k_prefabHierarchyNodeUUIDJsonKey);
 
            if (l_nodeUUID.is_nil()) { continue; }
 
            l_addedMap.try_emplace      (l_nodeUUID, &l_addedJson);
            l_addedUUIDList.emplace_back(l_nodeUUID);
        }
    }

    // 最終的な子の順序をUUID列で解決する
    // Removed済みUUIDはDeserializeRemovedUUIDListでHierarchyへ登録済み
    const auto& l_removedUUIDSet = a_gameObjectHierarchy.GetREFPrefabRemovedChildUUIDSet();

    // 自然順 = Prefab順(Removed除く) + Added順
    std::vector<boost::uuids::uuid> l_finalOrderUUIDList = {};

    for (const auto& l_prefabNodeJson : a_prefabChildListJson)
    {
        const auto& l_nodeUUID = Utility::DeserializeUUID(l_prefabNodeJson, k_prefabHierarchyNodeUUIDJsonKey);
 
        // NodeUUIDがNil値かRemovedUUIDに含まれているかで処理を飛ばす
        if (l_nodeUUID.is_nil() ||
            l_removedUUIDSet.contains(l_nodeUUID))
        {
            continue;
        }
 
        l_finalOrderUUIDList.emplace_back(l_nodeUUID);
    }
 
    for (const auto& l_nodeUUID : l_addedUUIDList)
    {
        l_finalOrderUUIDList.emplace_back(l_nodeUUID);
    }

    // OrderUUIDListがあれば保存時の順序を基礎にして、Prefab新規子をアンカー挿入する
    if (const auto& l_orderJsonArray = a_childListDiffJson.value(k_orderUUIDListJsonKey, nlohmann::json{});
        !l_orderJsonArray.is_null() &&
        Utility::IsJsonArray(l_orderJsonArray))
    {
        // OrderUUIDList順に、現在も有効なUUIDだけ並べる
        std::vector<boost::uuids::uuid>        l_resultOrderUUIDList = {};
        std::unordered_set<boost::uuids::uuid> l_placedUUIDSet       = {};
 
        for (const auto& l_json : l_orderJsonArray)
        {
            const auto& l_nodeUUID = Utility::DeserializeUUID(l_json, k_prefabHierarchyNodeUUIDJsonKey);
 
            if (l_nodeUUID.is_nil()) { continue; }
 
            // Prefab由来(Removed除く)かAddedか、どちらにも属さないUUIDは破棄
            const bool l_isPrefab = l_prefabNodeMap.contains(l_nodeUUID) &&
                                    !l_removedUUIDSet.contains(l_nodeUUID);

            const bool l_isAdded = l_addedMap.contains(l_nodeUUID);
 
            // プレハブ由来で追加された子ゲームオブジェクトでもなければ処理を飛ばす
            if (!l_isPrefab &&
                !l_isAdded)
            {
                continue;
            }
 
            l_resultOrderUUIDList.emplace_back(l_nodeUUID);
            l_placedUUIDSet.emplace           (l_nodeUUID);
        }
 
        // OrderUUIDListに無いPrefab新規の子をPrefab順でアンカー挿入
        boost::uuids::uuid l_anchorUUID = {};
 
        for (const auto& l_prefabNodeJson : a_prefabChildListJson)
        {
            const auto& l_nodeUUID = Utility::DeserializeUUID(l_prefabNodeJson, k_prefabHierarchyNodeUUIDJsonKey);
 
            // UUIDがNil値かRemovedUUIDに含まれているなら処理をスキップ
            if (l_nodeUUID.is_nil() ||
                l_removedUUIDSet.contains(l_nodeUUID))
            {
                continue;
            }
 
            // 既に配置済み = 順序確定済みなのでアンカーとして記憶する
            if (l_placedUUIDSet.contains(l_nodeUUID))
            {
                l_anchorUUID = l_nodeUUID;
 
                continue;
            }
 
            if (l_anchorUUID.is_nil())
            {
                // アンカーが無い = Prefab先頭への挿入
                l_resultOrderUUIDList.insert(l_resultOrderUUIDList.begin(), l_nodeUUID);
            }
            else
            {
                // アンカーの直後へ挿入
                const auto& l_anchorItr = std::find(l_resultOrderUUIDList.begin(), l_resultOrderUUIDList.end(), l_anchorUUID);
 
                l_resultOrderUUIDList.insert(std::next(l_anchorItr), l_nodeUUID);
            }
 
            // 挿入した子自身を次のアンカーにする(連続挿入の順序維持)
            l_anchorUUID = l_nodeUUID;
 
            l_placedUUIDSet.emplace(l_nodeUUID);
        }
 
        // OrderUUIDListに無いAddedを末尾へ(保険。通常は必ず含まれる)
        for (const auto& l_nodeUUID : l_addedUUIDList)
        {
            if (l_placedUUIDSet.contains(l_nodeUUID)) { continue; }
 
            l_resultOrderUUIDList.emplace_back(l_nodeUUID);
        }
 
        l_finalOrderUUIDList = std::move(l_resultOrderUUIDList);
    }

    // 確定した順序で1件ずつ子を生成・接続する
    // ConnectParentForDeserializeの呼び出し順 = 子リストの並びになる
    for (const auto& l_nodeUUID : l_finalOrderUUIDList)
    {
        if (const auto& l_prefabItr = l_prefabNodeMap.find(l_nodeUUID);
            l_prefabItr != l_prefabNodeMap.end())
        {
            // Prefab由来の子はPrefab側Jsonを基底にModified差分を渡す
            // Modifiedにエントリが無い = 差分なし(Prefabに完全追従)
            if (const auto& l_itr = l_modifiedDataMap.find(l_nodeUUID);
                l_itr != l_modifiedDataMap.end())
            {
                const auto* l_modifiedJson = l_itr->second;

                if (!l_modifiedJson) { continue; }
                
                const auto& l_modifiedGameObjectDataJson = l_modifiedJson->value(k_gameObjectDataJsonKey, nlohmann::json{});

                DeserializeChild(l_parent,
                                 l_modifiedGameObjectDataJson,
                                 *l_prefabItr->second,
                                 a_prefabSystem,
                                 a_scene);
            }
            else
            {
                DeserializeChild(l_parent,
                                 nlohmann::json{},
                                 *l_prefabItr->second,
                                 a_prefabSystem,
                                 a_scene);
            }

            continue;
        }
 
        // Addedの子はシーン側フルJsonをそのまま読む
        if (const auto& l_addedItr = l_addedMap.find(l_nodeUUID);
            l_addedItr != l_addedMap.end())
        {
            DeserializeChild(l_parent,
                             *l_addedItr->second,
                             nlohmann::json{},
                             a_prefabSystem,
                             a_scene);
        }
    }
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeChild(const std::weak_ptr<GameObject>&   a_parentGameObject, 
                                                                        const nlohmann::json&              a_childJson, 
                                                                        const nlohmann::json&              a_baseJson, 
                                                                        const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                              Scene&                       a_scene) const
{
    if (a_parentGameObject.expired())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "親GameObjectが無効なため、子のデシリアライズをスキップしました。");
 
        return;
    }
 
    // Owner設定等の初期化が必要なのでmake_sharedで生成してINITを呼ぶ
    auto l_child = std::make_shared<GameObject>();
 
    l_child->INIT();
 
    // PrefabHierarchyNodeUUIDは接続前に設定する
    // AddChildのNodeUUIDがNilだと新規発行してしまい照合が壊れるため
    auto l_nodeUUID = Utility::DeserializeUUID(a_childJson, k_prefabHierarchyNodeUUIDJsonKey);
 
    // 差分形式の場合nodeUUIDはPrefab基底側にある
    if (l_nodeUUID.is_nil())
    {
        l_nodeUUID = Utility::DeserializeUUID(a_baseJson, k_prefabHierarchyNodeUUIDJsonKey);
    }
 
    // 子にPrefabHierarchyNodeUUIDがしっかりと保存されていなければ
    // こちら側でデシリアライズしたNodeUUIDを格納しておく
    if (!l_nodeUUID.is_nil())
    {
        l_child->SetPrefabHierarchyNodeUUID(l_nodeUUID);
    }
 
    // 親子リンクを張る
    // この呼び出し順がそのまま親の子リストの並びになる
    // Transform::ApplyParentは呼ばないDeserialize専用の接続
    auto& l_hierarchy = l_child->GetMutableREFHierarchy();

    l_hierarchy.ConnectParentForDeserialize(a_parentGameObject);
 
    // GameObject本体のデシリアライズ
    // a_baseJson有りの場合はPrefab由来の子は基底Json + 差分JsonをGameObject側でマージして読む
    // a_baseJson無しの場合はAdded子はa_childJsonをフル形式で読む
    // SceneへのAddGameObject登録と子孫の再帰読み込みはGameObject側が行う
    l_child->DeserializeScene(a_childJson,
                              a_baseJson,
                              a_scene,
                              a_prefabSystem);
}

void FWK::Converter::GameObjectHierarchyJsonConverter::DeserializeRemovedUUIDList(const nlohmann::json& a_childListDiffJson, GameObjectHierarchy& a_gameObjectHierarchy) const
{
    const auto& l_rootJsonArray = a_childListDiffJson.value(k_removedUUIDListJsonKey, nlohmann::json{});

    if (!l_rootJsonArray.is_array()) { return; }

    for (const auto& l_json : l_rootJsonArray)
    {
        if (l_json.is_null()) { continue; }

        // {"RemovedUUID": "..."} から読み取る
        const auto& l_removedUUID = Utility::DeserializeUUID(l_json, k_removedUUIDJsonKey);

        if (l_removedUUID.is_nil()) { continue; }

        // Hierarchy側の削除UUIDセットへ登録する
        // AddChildが削除UUIDとの重複をはじき、DeserializeChildListDiffの
        // 順序解決でもRemoved除外に使う情報になる
        a_gameObjectHierarchy.AddPrefabRemovedUUID(l_removedUUID);
    }
}

nlohmann::json FWK::Converter::GameObjectHierarchyJsonConverter::SerializeRemovedUUIDList(const std::unordered_set<boost::uuids::uuid>&a_removedUUIDSet) const
{
    auto l_jsonArray = nlohmann::json::array();

    for (const auto& l_uuid : a_removedUUIDSet)
    {
        // Nil値のUUIDはまともに機能しないためcontinue
        if (l_uuid.is_nil()) { continue; }

        // {"RemovedUUID": "..."} 形式で出力する(DeserializeRemovedUUIDListと対称)
        l_jsonArray.emplace_back(Utility::SerializeUUID(l_uuid, k_removedUUIDJsonKey));
    }

    return l_jsonArray;
}