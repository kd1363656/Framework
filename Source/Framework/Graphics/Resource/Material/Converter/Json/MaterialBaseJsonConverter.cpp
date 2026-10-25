#include "MaterialBaseJsonConverter.h"

bool FWK::Converter::MaterialBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, const Graphics::MaterialBase& a_materialBase) const
{
    if (a_rootJson.is_null()) { return false; }

    const auto& l_typeINFO = a_materialBase.GetREFRuntimeTypeINFO();

    // 型名が書かれていないjsonは、型を確かめられないため読み込まない
    const auto& l_materialTypeName = a_rootJson.value(k_materialTypeJsonKey, std::string{});

    return l_materialTypeName == l_typeINFO.k_name;
}

nlohmann::json FWK::Converter::MaterialBaseJsonConverter::Serialize(const Graphics::MaterialBase& a_materialBase) const
{
    const auto& l_typeINFO = a_materialBase.GetREFRuntimeTypeINFO();

    nlohmann::json l_rootJson = {};

    // 読み込み時に、どの型のMaterialのjsonかを確かめるために型名を保存する
    l_rootJson[k_materialTypeJsonKey] = std::string{ l_typeINFO.k_name };

    return l_rootJson;
}