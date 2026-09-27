#include "GameObjectComponentContainerJsonConverter.h"

void FWK::Converter::GameObjectComponentContainerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer)
{
    if (a_rootJson.is_null()) { return; }

    // ComponentListが存在しない、または配列でないなら復元対象なし
    if (!Utility::IsJsonArray(a_rootJson, k_componentListJsonKey)) { return; }
 
    for (const auto& l_elementJson : a_rootJson[k_componentListJsonKey])
    {
        if (!l_elementJson.is_object()) { continue; }
 
        // 型名からファクトリー経由で生成する
        std::shared_ptr<ComponentBase> l_component = {};
 
        Utility::DeserializeInstanceType<TypeAlias::ComponentSharedFactory>(l_elementJson, k_componentTypeJsonKey, l_component);
 
        if (!l_component)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ComponentTypeに対応するコンポーネントをFactoryから生成できませんでした。");
 
            continue;
        }
 
        const auto& l_componentJson = l_elementJson.value(k_componentDataJsonKey, nlohmann::json{});
 
        // デシリアライズでデータを復元
        l_component->Deserialize(l_componentJson);
 
        // UUIDの保持・再発行判定はコンテナ側の登録経路に委ねる
        a_gameObjectComponentContainer.AddComponent(l_component);
    }
}

nlohmann::json FWK::Converter::GameObjectComponentContainerJsonConverter::Serialize(const GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    nlohmann::json l_rootJson          = {};
    nlohmann::json l_componentListJson = nlohmann::json::array();
 
    const auto& l_componentList = a_gameObjectComponentContainer.GetREFComponentList().GetREFElementDataList();
 
    for (const auto& l_componentData : l_componentList)
    {
        const auto& l_component = l_componentData.m_type;
 
        if (!l_component) { continue; }
 
        nlohmann::json l_componentJson = {};
 
        Utility::UpdateJson(l_componentJson, l_component->Serialize());
 
        nlohmann::json l_elementJson = {};
 
        Utility::UpdateJson(l_elementJson, Utility::SerializeInstanceType(l_component, k_componentTypeJsonKey));
 
        // シリアライズでデータを復元
        l_elementJson[k_componentDataJsonKey] = std::move(l_componentJson);
 
        l_componentListJson.emplace_back(std::move(l_elementJson));
    }
 
    l_rootJson[k_componentListJsonKey] = std::move(l_componentListJson);
 
    return l_rootJson;
}