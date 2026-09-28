#include "GameObjectComponentContainerJsonConverter.h"

void FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson, Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey))
    {
        return; 
    }
 
    for (const auto& l_elementJson : a_rootJson[Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey])
    {
        if (l_elementJson.is_null()) { continue; }
 
        std::shared_ptr<GameObjectComponentBase> l_component = {};
 
        // 生成すべきコンポーネントを生成
        Utility::DeserializeInstanceType<TypeAlias::GameObjectComponentSharedFactory>(l_elementJson, Constant::k_gameObjectComponentContainerJsonConverterComponentTypeJsonKey, l_component);
 
        if (!l_component)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ComponentTypeに対応するコンポーネントをFactoryから生成できませんでした。");
 
            continue;
        }
 
        const auto& l_componentJson = l_elementJson.value(Constant::k_gameObjectComponentContainerJsonConverterComponentDataJsonKey, nlohmann::json{});
 
        l_component->Deserialize(l_componentJson);
 
        // コンポーネントコンテナに追加
        a_gameObjectComponentContainer.AddComponent(l_component);
    }
}
void FWK::Converter::GameObjectComponentContainerJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, GameObjectComponentContainer& a_gameObjectComponentContainer) const
{
    if (a_rootJson.is_null() ||
        !Utility::IsJsonArray(a_rootJson, Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey))
    {
        return; 
    }
 
    for (const auto& l_json : a_rootJson[Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey])
    {
        if (l_json.is_null()) { continue; }
 
        std::shared_ptr<GameObjectComponentBase> l_component = {};
 
        Utility::DeserializeInstanceType<TypeAlias::GameObjectComponentSharedFactory>(l_json, Constant::k_gameObjectComponentContainerJsonConverterComponentTypeJsonKey, l_component);
 
        if (!l_component)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ComponentTypeに対応するコンポーネントをFactoryから生成できませんでした。");
 
            continue;
        }
 
        const auto& l_componentJson = l_json.value(Constant::k_gameObjectComponentContainerJsonConverterComponentDataJsonKey, nlohmann::json{});
 
        l_component->Deserialize(l_componentJson);
 
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
 
        nlohmann::json l_json = {};
 
        Utility::UpdateJson(l_json, Utility::SerializeInstanceType(l_component, Constant::k_gameObjectComponentContainerJsonConverterComponentTypeJsonKey));
 
        // シリアライズでデータを復元
        l_json[Constant::k_gameObjectComponentContainerJsonConverterComponentDataJsonKey] = std::move(l_componentJson);
 
        l_componentListJson.emplace_back(std::move(l_json));
    }
 
    l_rootJson[Constant::k_gameObjectComponentContainerJsonConverterComponentListJsonKey] = std::move(l_componentListJson);
 
    return l_rootJson;
}