#include "ComponentBaseJsonConverter.h"

void FWK::Converter::ComponentBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, ComponentBase& a_componentBase) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_uuid = Utility::DeserializeUUID(a_rootJson, k_uuidJsonKey);

    if (l_uuid.is_nil()) 
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントにUUIDが割り当てられていませんでした。");
    }
    else
    {
        a_componentBase.SetUUID(l_uuid);
    }

    const bool l_isDisable      = a_rootJson.value(k_isDisableJsonKey,      Constant::k_componentBaseInitialValueIsDisable);
    const bool l_isPrefabOrigin = a_rootJson.value(k_isPrefabOriginJsonKey, Constant::k_componentBaseInitialValueIsPrefabOrigin);

    a_componentBase.SetIsDisable     (l_isDisable);
    a_componentBase.SetIsPrefabOrigin(l_isPrefabOrigin);
}

nlohmann::json FWK::Converter::ComponentBaseJsonConverter::Serialize(const ComponentBase& a_componentBase) const
{
    nlohmann::json l_rootJson = {};

    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_componentBase.GetREFUUID(), k_uuidJsonKey));

    l_rootJson[k_isDisableJsonKey]      = a_componentBase.GetVALIsDisable     ();
    l_rootJson[k_isPrefabOriginJsonKey] = a_componentBase.GetVALIsPrefabOrigin();

    return l_rootJson;
}