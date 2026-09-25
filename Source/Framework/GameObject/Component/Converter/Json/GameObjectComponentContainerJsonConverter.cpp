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

        // Prefab由来のComponentであることを記録
        // Scene保存時に差分形式で出力するか完全データで出力するかの判定に使用する
        l_component->SetIsPrefabOrigin(true);
 
        // コンポーネントコンテナにPrefab由来のコンポーネントとして追加
        a_gameObjectComponentContainer.Add(l_component);
    }
 
    return true;

}
bool FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, const nlohmann::json& a_prefabComponentListJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    if (a_rootJson.is_null()) { return false; }

    // この時点でコンテナに登録されているのはDeserializePrefabで構築したPrefab由来Componentのみ
    // この後Scene追加分をAddすると内部リストは伸びるが
    // GetVALIsPrefabOrigin()ではじくためコピーを取らず参照のまま使う
    const auto& l_componentSmartPointerVectorList = a_gameObjectComponentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_componentDataList               = l_componentSmartPointerVectorList.GetREFElementDataList             ();
          
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

    // RemovedUUIDと同じUUIDを持つコンポーネントを削除予定として
    for (const auto& l_componentData : l_componentDataList)
    {
        const auto& l_component = l_componentData.m_type;

        a_gameObjectComponentContainer.ContainsRemovedComponentUUID(l_component->GetREFUUID());

        // コンポーネントが無効化かUUIDが登録されていなければreturn
        if (!l_component ||
            a_gameObjectComponentContainer.ContainsRemovedComponentUUID(l_component->GetREFUUID()))
        {
            continue;
        }

        l_component->SetMarkedForRemoval(true);
    }

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