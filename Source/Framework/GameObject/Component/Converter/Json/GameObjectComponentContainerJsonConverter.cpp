#include "GameObjectComponentContainerJsonConverter.h"

bool FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    if (a_rootJson.is_null()) { return false; }
 
    // ComponentListキーがない、または配列でなければ読み込む者がないので正常終了
    const auto& l_componentJsonArray = a_rootJson.value(Constant::k_gameObjectComponentListJsonKey, nlohmann::json{});
 
    // 配列JSONじゃない場合return
    if (!Utility::IsJsonArray(l_componentJsonArray)) { return true; }
 
    // プレハブに保存されているコンポーネントを
    // 保存されている順番のままコンテナへ登録する
    for (const auto& l_componentJson : l_componentJsonArray)
    {
        std::shared_ptr<ComponentBase> l_component = nullptr;

        // TypeINFOからコンポーネントを復元する
        Utility::DeserializeInstanceType<TypeAlias::ComponentSharedFactory>(l_componentJson, k_componentTypeNameJsonKey, l_component);
 
        if (!l_component) { return false; }
 
        l_component->INIT                 ();
        l_component->DeserializePrefabUUID(l_componentJson);
        l_component->DeserializePrefab    (l_componentJson);

        // コンポーネントコンテナにPrefab由来のコンポーネントとして追加
        a_gameObjectComponentContainer.AddPrefabComponent(l_component);
    }
 
    return true;

}
bool FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, const nlohmann::json& a_prefabComponentListJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    if (a_rootJson.is_null()) { return false; }

    // この時点でコンテナに登録されているのはDeserializePrefabで構築したPrefab由来Componentのみ
    // この後Scene追加分をAddすると内部リストは伸びるが
    // GetVALIsPrefabOrigin()ではじくためコピーを取らず参照のまま使う
          auto& l_componentSmartPointerVectorList = a_gameObjectComponentContainer.GetMutableREFComponentSmartPointerVectorList();
    const auto& l_componentDataList               = l_componentSmartPointerVectorList.GetREFElementDataList                    ();
          
    // Prefab由来Componentの削除予約を先に行う
    // 照合対象から確実に除外するため、照合ループより先にマークする
    if (const auto& l_removedJsonArray = a_rootJson.value(k_removedComponentUUIDListJsonKey, nlohmann::json{});
        Utility::IsJsonArray(l_removedJsonArray))
    {
        for (const auto& l_json : l_removedJsonArray)
        {
            // 配列の各要素は{ "UUID" : "..." } 形式のオブジェクトとして保存されている
            const auto& l_componentUUID = Utility::DeserializeUUID(l_json);

            if (l_componentUUID.is_nil()) { continue; }

            // 次回保存時に削除状態を維持するためランタイム側の削除記録へ登録する
            a_gameObjectComponentContainer.AddRemovedComponentUUID(l_componentUUID);
        }
    }

    // 削除記録に乗っているPrefab由来Componentへ削除予約をつける
    // 照合対象から確実に除外するため、照合ループより先にマークする
    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        // コンポーネントが無効化かUUIDが登録されていなければreturn
        if (!l_component ||
            !a_gameObjectComponentContainer.ContainsRemovedComponentUUID(l_component->GetREFUUID()))
        {
            continue;
        }

        l_component->SetMarkedForRemoval(true);
    }

    // SceneのComponentListを配列順に処理して順序・差分・追加分を復元する
    const auto& l_componentJsonArray = a_rootJson.value(Constant::k_gameObjectComponentListJsonKey, nlohmann::json{});

    std::vector<std::shared_ptr<ComponentBase>> l_orderedComponentList = {};
    std::unordered_set<ComponentBase*>          l_consumedAddressSet   = {};

    // 最終的な要素数の上限はSceneエントリ数 + Prefab由来の残存分
    l_orderedComponentList.reserve(l_componentJsonArray.size() + l_componentDataList.size());

    for (const auto& l_componentJson : l_componentJsonArray)
    {
        if (l_componentJson.is_null()) { continue; }

        // エントリのUUIDがPrefabゆらいComponentとの照合キー
        const auto l_componentUUID = Utility::DeserializeUUID(l_componentJson);

        if (l_componentUUID.is_nil()) { continue; }

        // UUIDレジストリでPrefabゆらいComponentのみ照合する
        // 削除済み墓標・Scene追加分はnullptrが帰りScene追加分へ流れる
        auto l_component = a_gameObjectComponentContainer.FindVALComponentFromUUID(l_componentUUID).lock();

        if (l_component)
        {
            // 削除済み・重複エントリ・Scene追加分のUUID一致は無視する
            if (l_component->GetVALIsMarkedForRemoval() ||
                !l_consumedAddressSet.emplace(l_component.get()).second)
            {
                continue;
            }

            // "Prefab"キーに完全データがある場合はそのまま復元する
            // (保存時にPrefab側エントリが見つからなかった場合っもフォールバック分)
            if (const auto& l_prefabDataJson = l_componentJson.value(k_componentPrefabJsonKey, nlohmann::json{});
                !l_prefabDataJson.is_null())
            {
                l_component->DeserializePrefab(l_prefabDataJson);
            }
            // "Scene"キーにはPrefab値との差分がオブジェクト形式で入っている
            // ベースラインとなるPrefab側データへmerge_patchで適用して復元する
            else if (const auto& l_sceneJson = l_componentJson.value(k_componentSceneJsonKey, nlohmann::json{});
                     !l_sceneJson.is_null())
            {
                nlohmann::json l_baselineJson = {};

                // 差分のベースラインはPrefab側の同UUIDエントリ
                if (const auto* l_prefabComponentJson = FindPTRPrefabComponentJson(a_prefabComponentListJson, l_componentUUID);
                    l_prefabComponentJson)
                {
                    l_baselineJson = *l_prefabComponentJson;
                }
                // 見つからない場合は現在値(Prefab適用済み状態)をベースラインにする
                else
                {
                    l_baselineJson = l_component->SerializePrefab();
                }

                // merge_patchはオブジェクトを再帰的にマージするため
                // 差分に含まれないフィールドはPrefab値が維持される
                l_baselineJson.merge_patch(l_sceneJson);

                l_component->DeserializePrefab(l_baselineJson);

                // "Scene"も"Prefab"もないエントリは変更なし
                // 順序復元のためだけに存在し、Prefab適用済みの状態を維持する
            }
        }
        else
        {
            // 照合失敗 : シーン側で追加されたComponent
            // ComponentTypeNameからファクトリ経由で生成する
            Utility::DeserializeInstanceType<TypeAlias::ComponentSharedFactory>(l_componentJson, k_componentTypeNameJsonKey, l_component);

            if (!l_component) { return false; }

            l_component->INIT                 ();
            l_component->DeserializePrefabUUID(l_componentJson);

            // Scene追加分は復元に必要な完全データが"Prefab"キーに入っている
            const auto& l_prefabDataJson = l_componentJson.value(k_componentPrefabJsonKey, nlohmann::json{});

            l_component->DeserializePrefab(l_prefabDataJson);

            // 型マップ・UUIDレジストリ・リストへすべて登録するコンテナのAddを使う
            a_gameObjectComponentContainer.AddSceneComponent(l_component);
        }

        // 処理したComponentおorderedListへ積む
        // 子の積む順番がSceneファイルの並び順そのもの
        l_orderedComponentList.emplace_back(l_component);
    }

    // Scene側に列挙されなかったPrefabゆらいComponentは
    // Scene保存後にPrefabへ追加されたものとして末尾へ残す
    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        if (!l_component                            ||
            !l_component->GetVALIsPrefabOrigin()    ||
            l_component->GetVALIsMarkedForRemoval() ||
            l_consumedAddressSet.contains(l_component.get()))
        {
            continue;
        }

        l_orderedComponentList.emplace_back(l_component);
    }

    // 削除予防Componentをここで確定削除する
    a_gameObjectComponentContainer.SweepRemoved();

    // 組み立てた順序でリストを再構築する
    // コンテナのAddではなく内部のリストのAddを使う
    // コンテナのAddは型マップ・UUIDレジストリへの二重登録で失敗するため    
    l_componentSmartPointerVectorList.Clear();

    for (const auto& l_component : l_orderedComponentList)
    {
        l_componentSmartPointerVectorList.Add(l_component);
    }

    return true;
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializePrefab(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return nlohmann::json();
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeScene(const nlohmann::json& a_prefabComponentListJson, const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return nlohmann::json();
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::DiffJson(const nlohmann::json& a_baseJson, const nlohmann::json& a_currentJson) const
{
    return nlohmann::json();
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::SerializeRemovedUUIDList(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    return nlohmann::json();
}

const nlohmann::json* FWK::Converter::GameObjectComponentContainerJsonConverter::FindPTRPrefabComponentJson(const nlohmann::json& a_prefabComponentListJson, const boost::uuids::uuid& a_componentUUID) const
{
    return nullptr;
}