#include "GameObjectComponentBaseJsonConverter.h"

void FWK::Converter::ComponentBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectComponentBase& a_gameObjectComponentBase) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_uuid = Utility::DeserializeUUID(a_rootJson, k_uuidJsonKey);

    if (l_uuid.is_nil()) 
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントにUUIDが割り当てられていませんでした。");
    }
    else
    {
        a_gameObjectComponentBase.SetUUID(l_uuid);
    }

    const bool l_isDisable      = a_rootJson.value(k_isDisableJsonKey,      Constant::k_gameObjectComponentBaseInitialValueIsDisable);
    const bool l_isPrefabOrigin = a_rootJson.value(k_isPrefabOriginJsonKey, Constant::k_gameObjectComponentBaseInitialValueIsPrefabOrigin);

    a_gameObjectComponentBase.SetIsDisable     (l_isDisable);
    a_gameObjectComponentBase.SetIsPrefabOrigin(l_isPrefabOrigin);
}

nlohmann::json FWK::Converter::ComponentBaseJsonConverter::Serialize(const GameObjectComponentBase& a_gameObjectComponentBase) const
{
    nlohmann::json l_rootJson = {};

    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObjectComponentBase.GetREFUUID(), k_uuidJsonKey));

    l_rootJson[k_isDisableJsonKey]      = a_gameObjectComponentBase.GetVALIsDisable     ();
    l_rootJson[k_isPrefabOriginJsonKey] = a_gameObjectComponentBase.GetVALIsPrefabOrigin();

    return l_rootJson;
}