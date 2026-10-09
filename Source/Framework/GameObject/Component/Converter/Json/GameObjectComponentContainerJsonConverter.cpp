#include "GameObjectComponentContainerJsonConverter.h"

void FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, const nlohmann::json& a_prefabJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    nlohmann::json l_sceneComponentListJson  = {};
    nlohmann::json l_prefabComponentListJson = {};

    if (!a_rootJson.is_null())
    {
        l_sceneComponentListJson = a_rootJson.value  (Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey, nlohmann::json{});
    }
    if (!a_prefabJson.is_null())
    {
        l_prefabComponentListJson = a_prefabJson.value(Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey, nlohmann::json{});
    }

    // Prefab基底のComponentListが存在し、
    // かつシーン側が差分構造(配列以外)ならPrefab+差分マージして読む
    // ※シーン側がnull(差分なし=Prefabに完全追従)でもこちらへ入る
    if (l_prefabComponentListJson.is_array() &&
        !l_sceneComponentListJson.is_array())
    {
        // 削除済みUUIDをコンテナのRemovedUUIDSetへ登録
        DeserializeRemovedUUIDList(l_sceneComponentListJson, a_gameObjectComponentContainer);

        // Prefab側へ差分適用してマージ済みリストを作る
        auto l_mergedListJson = l_prefabComponentListJson;

        // 差分を適用
        ApplyComponentListDiff(l_sceneComponentListJson, l_mergedListJson);

        // 差分を適用した状態でコンポーネントリストをデシリアライズ
        DeserializeComponentList(l_mergedListJson, a_gameObjectComponentContainer);

        return;
    }

    // プレハブが読み込めなければフル差分形式で読み込む
    DeserializeComponentList(l_sceneComponentListJson, a_gameObjectComponentContainer);
}
void FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    const auto& l_componentListJson = a_rootJson.value(Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey, nlohmann::json{});

    DeserializeComponentList(l_componentListJson, a_gameObjectComponentContainer);
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::Serialize(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    nlohmann::json l_rootJson = {};

    // コンポーネントリストをシリアライズ
    l_rootJson[Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey] = SerializeComponentList(a_gameObjectComponentContainer);

    return l_rootJson;
}
nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeDiff(const nlohmann::json& a_prefabJson, const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    const auto& l_rootJson = a_prefabJson.value(Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey, nlohmann::json{});

    // Prefab基底が無ければ差分は作れないので空の差分構造を返す
    if (!l_rootJson.is_array())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefab側ComponentListが無効なため、差分シリアライズに失敗しました。");

        nlohmann::json l_emptyDiffJson = {};

        l_emptyDiffJson[k_addedJsonKey]           = nlohmann::json::array();
        l_emptyDiffJson[k_removedUUIDListJsonKey] = nlohmann::json::array();
        l_emptyDiffJson[k_modifiedJsonKey]        = nlohmann::json::array();

        return l_emptyDiffJson;
    }

    // 現在の全コンポーネントをフル形式配列へ
    const auto& l_currentListJson = SerializeComponentList(a_gameObjectComponentContainer);

    // Prefab基底との差分を検出して返す
    return DetectComponentListDiff(l_rootJson, l_currentListJson);
}

void FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeComponentList(const nlohmann::json& a_componentListJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    if (!a_componentListJson.is_array()) { return; }

    for (const auto& l_json : a_componentListJson)
    {
        std::shared_ptr<GameObjectComponentBase> l_component = nullptr;

        // ComponentType名からFactoryで生成する
        Utility::DeserializeInstanceType<TypeAlias::GameObjectComponentSharedFactory>(l_json, k_componentTypeJsonKey, l_component);

        if (!l_component)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ComponentTypeの復元に失敗したため、コンポーネントのデシリアライズをスキップしました。");

            continue;
        }

        // ComponentData内にUUID/IsDisable/IsPrefabOrigin/各プロパティが含まれる
        l_component->Deserialize(l_json.value(k_componentDataJsonKey, nlohmann::json{}));

        // ComponentData内にUUIDが無い場合は外側のComponentUUIDで補完する
        if (const auto& l_componentUUID = l_component->GetREFUUID();
            l_componentUUID.is_nil())
        {
            l_component->SetUUID(Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey));
        }

        // UUID未発行・重複・削除済みUUIDはAddComponent側で新規発行される
        if (!a_gameObjectComponentContainer.AddComponent(l_component))
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ComponentContainerへの登録に失敗しました。");
        }
    }
}
void FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeRemovedUUIDList(const nlohmann::json& a_componentListDiffJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    const auto& l_rootJsonArray = a_componentListDiffJson.value(k_removedUUIDListJsonKey, nlohmann::json{});

    if (!l_rootJsonArray.is_array()) { return; }

    for (const auto& l_json : l_rootJsonArray)
    {
        if (l_json.is_null()) { continue; }

        // {"RemovedUUID": "..."} から読み取る
        const auto& l_removedUUID = Utility::DeserializeUUID(l_json, k_removedUUIDJsonKey);

        if (l_removedUUID.is_nil()) { continue; }

        // コンテナ側の削除UUIDセットへ登録する
        // AddComponentが削除UUIDとの重複をはじくための情報になる
        a_gameObjectComponentContainer.AddPrefabRemovedComponentUUID(l_removedUUID);
    }
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeComponentList(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
          auto  l_jsonArray                       = nlohmann::json::array                                               ();
    const auto& l_componentSmartPointerVectorList = a_gameObjectComponentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_componentDataList               = l_componentSmartPointerVectorList.GetREFElementDataList             ();

    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component) { continue; }

        nlohmann::json l_json = {};

        // Factory復元用の型名と照合用UUIDはComponentDataの外側へ
        Utility::UpdateJson(Utility::SerializeInstanceType(l_component, k_componentTypeJsonKey), l_json);
        Utility::UpdateJson(Utility::SerializeUUID(l_component->GetREFUUID(), k_componentUUIDJsonKey), l_json);

        // Component本体のSerialize結果(UUID, IsDisable, IsPrefabOrigin, 各種プロパティを含む)
        l_json[k_componentDataJsonKey] = l_component->Serialize();

        l_jsonArray.emplace_back(std::move(l_json));
    }

    return l_jsonArray;
}
nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeRemovedUUIDList(const std::unordered_set<boost::uuids::uuid>& a_removedUUIDSet) const
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

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::DetectComponentListDiff(const nlohmann::json& a_baseJson, const nlohmann::json& a_currentJson) const
{
    auto l_addedJsonArray    = nlohmann::json::array();
    auto l_modifiedJsonArray = nlohmann::json::array();

    // RemovedはUUIDのsetで集計してからシリアライズする
    std::unordered_set<boost::uuids::uuid>                        l_removedUUIDSet = {};

    // baseをUUIDで引けるようにする
    std::unordered_map<boost::uuids::uuid, const nlohmann::json*> l_baseMap        = {};
    std::unordered_set<boost::uuids::uuid>                        l_matchedUUIDSet = {};

    // a_baseJsonが配列ならUUIDからJsonの対応表を作成
    if (a_baseJson.is_array())
    {
        for (const auto& l_json : a_baseJson)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            if (l_uuid.is_nil()) { continue; }

            l_baseMap.try_emplace(l_uuid, &l_json);
        }
    }

    if (a_currentJson.is_array())
    {
        for (const auto& l_json : a_currentJson)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            if (l_uuid.is_nil()) { continue; }

            const auto& l_baseITR = l_baseMap.find(l_uuid);

            // baseに存在しない = インスタンス独自の追加コンポーネント
            if (l_baseITR == l_baseMap.end())
            {
                l_addedJsonArray.emplace_back(l_json);

                continue;
            }

            l_matchedUUIDSet.emplace(l_uuid);

            // ComponentData同士を比較して変更キーだけを差分にする
            const auto& l_dataDiffJson = Utility::DetectJsonDiff(l_baseITR->second->value(k_componentDataJsonKey, nlohmann::json{}), l_json.value(k_componentDataJsonKey, nlohmann::json{}));

            if (l_dataDiffJson.is_null()) { continue; }

            nlohmann::json l_modifiedJson = {};

            Utility::UpdateJson(Utility::SerializeUUID(l_uuid, k_componentUUIDJsonKey), l_modifiedJson);

            l_modifiedJson[k_componentDataJsonKey] = l_dataDiffJson;

            l_modifiedJsonArray.emplace_back(std::move(l_modifiedJson));
        }
    }

    // baseにあってcurrentに無いUUID = インスタンスで削除済み
    for (const auto& [l_uuid, l_json] : l_baseMap)
    {
        if (l_matchedUUIDSet.contains(l_uuid)) { continue; }

        l_removedUUIDSet.emplace(l_uuid);
    }

    // 順序差分の検出
    // 自然順 = Prefab順(Removed除く) + Added末尾
    std::vector<boost::uuids::uuid> l_naturalOrderUUIDList = {};
    std::vector<boost::uuids::uuid> l_currentOrderUUIDList = {};

    if (a_baseJson.is_array())
    {
        for (const auto& l_json : a_baseJson)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            if (l_uuid.is_nil() ||
                l_removedUUIDSet.contains(l_uuid))
            {
                continue;
            }

            l_naturalOrderUUIDList.emplace_back(l_uuid);
        }
    }

    if (a_currentJson.is_array())
    {
        for (const auto& l_json : a_currentJson)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            if (l_uuid.is_nil()) { continue; }

            // Prefabに無い = AddedのUUIDでもある
            if (!l_baseMap.contains(l_uuid))
            {
                l_naturalOrderUUIDList.emplace_back(l_uuid);
            }

            l_currentOrderUUIDList.emplace_back(l_uuid);
        }
    }

    nlohmann::json l_diffJson = {};

    l_diffJson[k_addedJsonKey]           = l_addedJsonArray;
    l_diffJson[k_removedUUIDListJsonKey] = SerializeRemovedUUIDList(l_removedUUIDSet);
    l_diffJson[k_modifiedJsonKey]        = l_modifiedJsonArray;

    // 自然順と現在順が異なる場合のみ順序を保存する
    // (一致するならPrefab側の順序変更をそのまま伝播させるため)
    if (l_naturalOrderUUIDList != l_currentOrderUUIDList)
    {
        auto l_orderJsonArray = nlohmann::json::array();

        for (const auto& l_uuid : l_currentOrderUUIDList)
        {
            l_orderJsonArray.emplace_back(Utility::SerializeUUID(l_uuid, k_componentUUIDJsonKey));
        }

        l_diffJson[k_orderUUIDListJsonKey] = std::move(l_orderJsonArray);
    }

    return l_diffJson;
}

void FWK::Converter::GameObjectComponentContainerJsonConverter::ApplyComponentListDiff(const nlohmann::json& a_diffJson, nlohmann::json& a_baseJson) const
{
    // baseが無効(null)なら空配列として扱う。Addedだけでも適用されるようにするため
    if (a_baseJson.is_null())
    {
        a_baseJson = nlohmann::json::array();
    }

    if (!Utility::IsJsonArray(a_baseJson)) { return; }

    const auto& l_removedJsonArray  = a_diffJson.value(k_removedUUIDListJsonKey, nlohmann::json::array());
    const auto& l_modifiedJsonArray = a_diffJson.value(k_modifiedJsonKey,        nlohmann::json::array());
    const auto& l_addedJsonArray    = a_diffJson.value(k_addedJsonKey,           nlohmann::json::array());

    // 削除されたコンポーネントを高速で調べるためのSet
    std::unordered_set<boost::uuids::uuid> l_removedUUIDSet = {};

    // 削除されたPrefabゆらいのコンポーネントをデシリアライズ
    for (const auto& l_json : l_removedJsonArray)
    {
        if (!l_json.is_object()) { continue; }

        const auto& l_removedUUID = Utility::DeserializeUUID(l_json, k_removedUUIDJsonKey);

        if (l_removedUUID.is_nil()) { continue; }

        l_removedUUIDSet.emplace(l_removedUUID);
    }

    // 変更があったコンポーネントをUUIDとJsonで対応付けるためのMap
    std::unordered_map<boost::uuids::uuid, const nlohmann::json*> l_modifiedMap = {};

    // Modified(変更のあったコンポーネント)をUUIDMapへ追加
    for (const auto& l_json : l_modifiedJsonArray)
    {
        const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

        if (l_uuid.is_nil()) { continue; }

        l_modifiedMap.try_emplace(l_uuid, &l_json);
    }

    // ベースJsonと変更の加わった内容を保持するための葉入れ宇
    auto l_mergedJsonArray = nlohmann::json::array();

    // base(Prefab側)を走査、Removedを飛ばしModifiedを差分適用する
    // Prefab側の新規追加コンポーネントもここで自動的に含まれる = 変更伝播
    for (const auto& l_json : a_baseJson)
    {
        const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

        // 取得したUUIDがシーン上で削除されたコンポーネントのものなら処理をスキップ
        if (l_uuid.is_nil() ||
            l_removedUUIDSet.contains(l_uuid))
        {
            continue;
        }

        nlohmann::json l_mergedComponentJson = l_json;

        if (const auto& l_modifiedITR = l_modifiedMap.find(l_uuid);
            l_modifiedITR != l_modifiedMap.end())
        {
            const auto* l_modifiedJson = l_modifiedITR->second;

            // 念のためヌルチェック
            if (!l_modifiedJson) { continue; }

            const auto& l_componentDataJson     = l_json.value         (k_componentDataJsonKey, nlohmann::json{});
            const auto& l_componentModifiedJson = l_modifiedJson->value(k_componentDataJsonKey, nlohmann::json{});

            // Prefabに保存されているコンポーネントデータとそのコンポーネントがシーンで変更された情報マージし
            // キーはコンポーネントデータとして保存
            l_mergedComponentJson[k_componentDataJsonKey] = Utility::ApplyJsonDiff(l_componentDataJson, l_componentModifiedJson);
        }

        // マージ結果を保存
        l_mergedJsonArray.emplace_back(std::move(l_mergedComponentJson));
    }

    // インスタンス独自コンポーネントを末尾へ
    for (const auto& l_json : l_addedJsonArray)
    {
        l_mergedJsonArray.emplace_back(l_json);
    }

    // OrderUUIDListがあればマージ済みリストをその順序へ並べ替える
    if (const auto& l_orderJsonArray = a_diffJson.value(k_orderUUIDListJsonKey, nlohmann::json{});
        l_orderJsonArray.is_array())
    {
        // マージ済みエントリをUUIDで引けるようにする
        std::unordered_map<boost::uuids::uuid, nlohmann::json> l_mergedEntryMap = {};

        for (auto& l_json : l_mergedJsonArray)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            if (l_uuid.is_nil()) { continue; }

            l_mergedEntryMap.try_emplace(l_uuid, std::move(l_json));
        }

        // OrderUUIDList(保存時の並び)の順に、現在も存在するUUIDだけ並べる
        std::vector<boost::uuids::uuid>        l_resultOrderUUIDList = {};
        std::unordered_set<boost::uuids::uuid> l_placedUUIDSet       = {};

        for (const auto& l_json : l_orderJsonArray)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            // マージされたマップに存在しないUUIDなら処理を飛ばす
            // 追加されたとしたらシーンJsonにかきこまれるべきだからおかしい
            if (l_uuid.is_nil() ||
                !l_mergedEntryMap.contains(l_uuid))
            {
                continue;
            }

            l_resultOrderUUIDList.emplace_back(l_uuid);
            l_placedUUIDSet.emplace           (l_uuid);
        }

        // OrderUUIDListに無いPrefab由来コンポーネント(Prefab側の後追い追加)を
        // Prefab順で直前の既知コンポーネントの直後へ挿入する
        boost::uuids::uuid l_anchorUUID = {};

        for (const auto& l_json : a_baseJson)
        {
            const auto& l_uuid = Utility::DeserializeUUID(l_json, k_componentUUIDJsonKey);

            if (l_uuid.is_nil()) { continue; }

            // 既に配置済み = 順序確定済みなのでアンカーとして記憶する
            if (l_placedUUIDSet.contains(l_uuid))
            {
                l_anchorUUID = l_uuid;

                continue;
            }

            // マージ済みに存在しない = Removed等なので対象外
            if (!l_mergedEntryMap.contains(l_uuid)) { continue; }

            if (l_anchorUUID.is_nil())
            {
                // アンカーが無い = Prefab先頭への挿入
                l_resultOrderUUIDList.insert(l_resultOrderUUIDList.begin(), l_uuid);
            }
            else
            {
                // アンカーの直後へ挿入
                const auto& l_anchorITR = std::find(l_resultOrderUUIDList.begin(), l_resultOrderUUIDList.end(), l_anchorUUID);

                l_resultOrderUUIDList.insert(std::next(l_anchorITR), l_uuid);
            }

            // 挿入したコンポーネント自身を次のアンカーにする
            // (Prefab側で連続追加されたコンポーネントの順序を維持するため)
            l_anchorUUID = l_uuid;

            l_placedUUIDSet.emplace(l_uuid);
        }

        // 3. 確定した順序でエントリを再配置する
        l_mergedJsonArray.clear();

        for (const auto& l_uuid : l_resultOrderUUIDList)
        {
            if (const auto& l_mergedEntryITR = l_mergedEntryMap.find(l_uuid);
                l_mergedEntryITR != l_mergedEntryMap.end())
            {
                l_mergedJsonArray.emplace_back(std::move(l_mergedEntryITR->second));
            }
        }
    }

    // ベースJsonに差分をマージしたJsonをMove
    a_baseJson = std::move(l_mergedJsonArray);
}