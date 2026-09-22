#include "ComponentBaseJsonConverter.h"

void FWK::Converter::ComponentBaseJsonConverter::DeserializePrefabUUID(const nlohmann::json& a_rootJson, ComponentBase& a_componentBase) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_uuid = Utility::DeserializeUUID(a_rootJson, k_uuidJsonKey);

    if (l_uuid.is_nil()) 
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントにUUIDが割り当てられていませんでした。");

        return;
    }

    a_componentBase.SetUUID(l_uuid);
}

nlohmann::json FWK::Converter::ComponentBaseJsonConverter::SerializePrefabUUID(const ComponentBase& a_componentBase) const
{
    nlohmann::json l_rootJson = {};

    Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_componentBase.GetREFUUID(), k_uuidJsonKey));

    return l_rootJson;
}