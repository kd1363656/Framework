#include "MaterialSystemJsonConverter.h"

void FWK::Converter::MaterialSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::MaterialSystem& a_materialSystem) const
{
    if (a_rootJson.is_null()) { return; }

    if (const auto& l_json = a_rootJson.value(k_materialStorageJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_materialStorage = a_materialSystem.GetMutableREFMaterialStorage();

        l_materialStorage.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::MaterialSystemJsonConverter::Serialize(const Graphics::MaterialSystem& a_materialSystem) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_materialStorage = a_materialSystem.GetREFMaterialStorage();

    l_rootJson[k_materialStorageJsonKey] = l_materialStorage.Serialize();

    return l_rootJson;
}