#include "GameObjectJsonConverter.h"

void FWK::Converter::GameObjectJsonConverter::DeserializeScene(const std::weak_ptr<GameObject>&   a_gameObject, 
                                                               const nlohmann::json&              a_rootJson,
                                                               const nlohmann::json&              a_baseJson, 
                                                               const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                     Scene&                       a_scene) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_gameObject = a_gameObject.lock();
 
    if (!l_gameObject) { return; }
 
    // const参照は宣言時に束ねた先から付け替えられないため、
    // 「引数のa_baseJson」か「PrefabSystemから引いたJson」のどちらかを
    // コピーせず指し替えられるようconstポインタで保持する
    const auto* l_baseJsonPtr = &a_baseJson; 
    const auto& l_prefabUUID  = Utility::DeserializeUUID(a_rootJson, k_prefabUUIDJsonKey);
 
    // 引数でPrefab基底が渡されず、PrefabUUIDがあればPrefabSystemから取得する
    // (Prefab内部子の差分経路ではa_baseJsonが渡されるのでここには入らない)
    if (a_baseJson.is_null() &&
        !l_prefabUUID.is_nil())
    {
        if (const auto* l_prefab = a_prefabSystem.FindPTRPrefab(l_prefabUUID);
            l_prefab)
        {
            l_baseJsonPtr = &l_prefab->GetREFJson();
        }
        else
        {
            FWK_ADD_LOG(Constant::k_imguiDebugINFOColor, "PrefabUUID : {}\nに対応するPrefabが見つからないため、フル形式として読み込みます。", boost::uuids::to_string(l_prefabUUID));
        }
    }
 
    // エントリ直下のメタ情報を復元
    // PrefabUUID / SceneInstanceUUIDは差分形式でも必ずエントリ直下にある
    if (!l_prefabUUID.is_nil())
    {
        l_gameObject->SetPrefabUUID(l_prefabUUID);
    }
 
    if (const auto& l_sceneInstanceUUID = Utility::DeserializeUUID(a_rootJson, k_sceneInstanceUUIDJsonKey);
        !l_sceneInstanceUUID.is_nil())
    {
        l_gameObject->SetSceneInstanceUUID(l_sceneInstanceUUID);
    }
 
    // Sceneへ登録
    // 子の読み込みより先に登録する(親→子の登録順が必要なため)
    // 実行レベル計算に親子関係が要るので呼び出し側が先に親接続を済ませている前提
    a_scene.AddGameObject(l_gameObject);
 
    // GameObjectレベルのフィールド(Name / IsPrefabOrigin等)
    DeserializeCommon(a_rootJson, *l_baseJsonPtr, *l_gameObject);
 
    // 差分形式なら各リストはDiffキー内、フル形式ならrootJson直下にある
    const auto& l_diffJson = a_rootJson.value  (k_diffJsonKey, nlohmann::json{});
    const auto& l_dataJson = l_diffJson.is_null() ? a_rootJson : l_diffJson;
 
    // 各Converterが自分のキーを読み、差分/フルを自前で振り分ける
    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();
    auto& l_hierarchy          = l_gameObject->GetMutableREFHierarchy         ();

    l_componentContainer.DeserializeScene(l_dataJson, *l_baseJsonPtr);

    l_hierarchy.DeserializeScene(l_dataJson,
                                 *l_baseJsonPtr, 
                                 a_prefabSystem,
                                 a_scene);
}
void FWK::Converter::GameObjectJsonConverter::DeserializePrefab(const std::weak_ptr<GameObject>&   a_gameObject, 
                                                                const nlohmann::json&              a_rootJson, 
                                                                const SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                                      Scene&                       a_scene) const
{
    if (a_rootJson.is_null()) { return; }
 
    const auto& l_gameObject = a_gameObject.lock();
 
    if (!l_gameObject) { return; }
 
    // エントリ直下のメタ情報を復元
    // Prefabファイル内の各ノードにはPrefabUUIDが保存されている
    // (ConvertToPrefabで同一PrefabUUIDが子へも伝播されている)
    if (const auto& l_prefabUUID = Utility::DeserializeUUID(a_rootJson, k_prefabUUIDJsonKey);
        !l_prefabUUID.is_nil())
    {
        l_gameObject->SetPrefabUUID(l_prefabUUID);
    }
 
    // Sceneへ登録
    // DeserializeSceneと同様、子の読み込みより先に登録する
    a_scene.AddGameObject(l_gameObject);
 
    // GameObjectレベルのフィールド
    // Prefab内ノードはフル形式なのでPrefab基底Jsonは存在しない(nullを渡す)
    DeserializeCommon(a_rootJson, nlohmann::json{}, *l_gameObject);
 
    auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();
    auto& l_hierarchy          = l_gameObject->GetMutableREFHierarchy         ();

    // Prefab内ノードは常にフル形式(配列)なのでPrefab専用の読み込み関数を使う
    l_componentContainer.DeserializePrefab(a_rootJson);
    l_hierarchy.DeserializePrefab         (a_rootJson, a_prefabSystem, a_scene);
}

nlohmann::json FWK::Converter::GameObjectJsonConverter::Serialize(const GameObject& a_gameObject, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    nlohmann::json l_rootJson = {};
 
    // 名前とプレハブかどうかでシリアライズ
    l_rootJson[k_nameJsonKey]           = a_gameObject.GetREFName          ();
    l_rootJson[k_isPrefabOriginJsonKey] = a_gameObject.GetVALIsPrefabOrigin();
 
    // UUID群のデシリアライズ
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabUUID(),              k_prefabUUIDJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabHierarchyNodeUUID(), k_prefabHierarchyNodeUUIDJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFSceneInstanceUUID(),       k_sceneInstanceUUIDJsonKey));
 
    // ComponentList / ChildListのデシリアライズ
    // 各Serialize()が {ComponentList:[...]} / {ChildList:[...]} を返すのでそのままマージする
    auto& l_componentContainer = a_gameObject.GetREFComponentContainer();
    auto& l_hierarchy          = a_gameObject.GetREFHierarchy         ();

    Utility::UpdateJson(l_rootJson, l_componentContainer.Serialize());
    Utility::UpdateJson(l_rootJson, l_hierarchy.Serialize(a_prefabSystem));
 
    return l_rootJson;
}
nlohmann::json FWK::Converter::GameObjectJsonConverter::SerializeScene(const GameObject& a_gameObject, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    const auto& l_prefabUUID = a_gameObject.GetREFPrefabUUID();
 
    // Prefab由来でなければフル形式
    if (l_prefabUUID.is_nil())
    {
        return Serialize(a_gameObject, a_prefabSystem);
    }
 
    // PrefabUUIDがあるならPrefabSystemから基底Jsonを引いて差分形式へ
    const auto* l_prefab = a_prefabSystem.FindPTRPrefab(l_prefabUUID);
 
    if (!l_prefab)
    {
        // 参照先Prefabが見つからない = 差分の基底が無いのでフル形式で保存する
        // (Prefab削除済み等。データを失わないよう完全なJsonを残す)
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabUUID : {}\nに対応するPrefabが見つからないため、フル形式でシリアライズします。", boost::uuids::to_string(l_prefabUUID));
 
        return Serialize(a_gameObject, a_prefabSystem);
    }
 
    return SerializeDif(l_prefab->GetREFJson(), a_gameObject, a_prefabSystem);
}
nlohmann::json FWK::Converter::GameObjectJsonConverter::SerializeDif(const nlohmann::json& a_baseJson, const GameObject& a_gameObject, SceneGameObjectPrefabSystem& a_prefabSystem) const
{
    // 基底が無効なら差分は作れないのでフル形式へフォールバック
    if (a_baseJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefab基底Jsonが無効なため、フル形式でシリアライズします。");
 
        return Serialize(a_gameObject, a_prefabSystem);
    }
 
    nlohmann::json l_diffJson = {};
 
    // GameObjectレベルフィールドの差分
    // ComponentList/ChildListは独自の差分形式を持つので生のJsonDiffでは処理しない
    // 基底と値が異なる場合のみDiffへ書き出す(未変更ならPrefab追従)
 
    // Name
    if (const auto& l_baseJsonName = a_baseJson.value(k_nameJsonKey, std::string{});
        l_baseJsonName != a_gameObject.GetREFName())
    {
        l_diffJson[k_nameJsonKey] = a_gameObject.GetREFName();
    }
 
    // IsPrefabOrigin
    if (const bool l_isPrefabOrigin = a_baseJson.value(k_isPrefabOriginJsonKey, Constant::l_gameObjectInitialValueIsPrefabOrigin); 
        l_isPrefabOrigin != a_gameObject.GetVALIsPrefabOrigin())
    {
        l_diffJson[k_isPrefabOriginJsonKey] = a_gameObject.GetVALIsPrefabOrigin();
    }
 
    // ComponentList / ChildList の差分
    // 基底Jsonを丸ごと渡し、各Converterが自分のキーを読んで差分を検出する
    auto& l_componentContainer = a_gameObject.GetREFComponentContainer();
    auto& l_hierarchy          = a_gameObject.GetREFHierarchy         ();

    l_diffJson[Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey] = l_componentContainer.SerializeDiff(a_baseJson); 
    l_diffJson[Constant::k_gameObjectHierarchyJsonConverterChildListJsonKey]              = l_hierarchy.SerializeDiff         (a_baseJson, a_prefabSystem);
 
    // エントリ直下のメタ情報
    nlohmann::json l_rootJson = {};
 
    // PrefabUUID: DeserializeSceneでのPrefab検索に使う
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabUUID(), k_prefabUUIDJsonKey));
 
    // SceneInstanceUUID: インスタンス識別用。これが無いとロード毎に新規発行され外部参照が壊れる
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFSceneInstanceUUID(), k_sceneInstanceUUIDJsonKey));
 
    // PrefabHierarchyNodeUUID: 子として配置される場合の照合用
    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabHierarchyNodeUUID(), k_prefabHierarchyNodeUUIDJsonKey));
 
    l_rootJson[k_diffJsonKey] = std::move(l_diffJson);
 
    return l_rootJson;
}

void FWK::Converter::GameObjectJsonConverter::DeserializeCommon(const nlohmann::json& a_rootJson, const nlohmann::json& a_baseJson, GameObject& a_gameObject) const
{
    // フィールド解決用にJsonを優先順位でマージ
    // 優先順位: Diff(シーン差分) > rootJson直下(フル形式/メタ情報) > baseJson(Prefab基底)
    // 先に低い優先度を入れて後から高い優先度で上書きする
    nlohmann::json l_mergedJson = a_baseJson.is_object() ? a_baseJson : nlohmann::json{};
 
    if (a_rootJson.is_object())
    {
        Utility::UpdateJson(l_mergedJson, a_rootJson);
    }
 
    if (const auto& l_diffJson = a_rootJson.value(k_diffJsonKey, nlohmann::json{});
        l_diffJson.is_object())
    {
        Utility::UpdateJson(l_mergedJson, l_diffJson);
    }
 
    // 名前のデシリアライズ
    if (const auto& l_name = l_mergedJson.value(k_nameJsonKey, std::string{});
        !l_name.empty())
    {
        a_gameObject.SetName(l_name);
    }
 
    const bool l_isPrefabOrigin = l_mergedJson.value(k_isPrefabOriginJsonKey, Constant::l_gameObjectInitialValueIsPrefabOrigin);
 
    a_gameObject.SetIsPrefabOrigin(l_isPrefabOrigin);

    // PrefabHierarchyNodeUUIDのデシリアライズ
    // 子経路ではDeserializeChildが接続前に設定済みだが、
    // シーン直置きGameObject等、こちら経由でのみUUIDが来る経路用の保険
    if (const auto& l_nodeUUID = Utility::DeserializeUUID(l_mergedJson, k_prefabHierarchyNodeUUIDJsonKey);
        !l_nodeUUID.is_nil())
    {
        a_gameObject.SetPrefabHierarchyNodeUUID(l_nodeUUID);
    }
}