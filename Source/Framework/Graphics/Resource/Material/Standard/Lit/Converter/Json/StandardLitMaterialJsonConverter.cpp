#include "StandardLitMaterialJsonConverter.h"

bool FWK::Converter::StandardLitMaterialJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardLitMaterial& a_standardLitMaterial) const
{
    // 型名が違うjsonの値は読まない
    if (!a_standardLitMaterial.MaterialBase::Deserialize(a_rootJson)) { return false; }

    a_standardLitMaterial.SetBaseColorFactor(Utility::DeserializeColor(a_rootJson, k_baseColorFactorJsonKey));
    a_standardLitMaterial.SetRoughnessFactor(a_rootJson.value(k_roughnessFactorJsonKey, Constant::k_defaultStandardMaterialRoughnessFactor));
    a_standardLitMaterial.SetMetallicFactor (a_rootJson.value(k_metallicFactorJsonKey,  Constant::k_defaultStandardMaterialMetallicFactor));

    a_standardLitMaterial.SetBaseColorTextureFilePath(Utility::DeserializeWString(a_rootJson, k_baseColorTextureFilePathJsonKey));
    a_standardLitMaterial.SetNormalTextureFilePath   (Utility::DeserializeWString(a_rootJson, k_normalTextureFilePathJsonKey));
    a_standardLitMaterial.SetRoughnessTextureFilePath(Utility::DeserializeWString(a_rootJson, k_roughnessTextureFilePathJsonKey));
    a_standardLitMaterial.SetMetallicTextureFilePath (Utility::DeserializeWString(a_rootJson, k_metallicTextureFilePathJsonKey));

    return true;
}

nlohmann::json FWK::Converter::StandardLitMaterialJsonConverter::Serialize(const Graphics::StandardLitMaterial& a_standardLitMaterial) const
{
    auto l_rootJson = a_standardLitMaterial.MaterialBase::Serialize();

    const auto& l_baseColorFactor          = a_standardLitMaterial.GetREFBaseColorFactor         ();
    const auto& l_baseColorTextureFilePath = a_standardLitMaterial.GetREFBaseColorTextureFilePath();
    const auto& l_normalTextureFilePath    = a_standardLitMaterial.GetREFNormalTextureFilePath   ();
    const auto& l_roughnessTextureFilePath = a_standardLitMaterial.GetREFRoughnessTextureFilePath();
    const auto& l_metallicTextureFilePath  = a_standardLitMaterial.GetREFMetallicTextureFilePath ();

    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_baseColorFactor, k_baseColorFactorJsonKey));

    l_rootJson[k_roughnessFactorJsonKey] = a_standardLitMaterial.GetVALRoughnessFactor();
    l_rootJson[k_metallicFactorJsonKey]  = a_standardLitMaterial.GetVALMetallicFactor ();

    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_baseColorTextureFilePath, k_baseColorTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_normalTextureFilePath,    k_normalTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_roughnessTextureFilePath, k_roughnessTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_metallicTextureFilePath,  k_metallicTextureFilePathJsonKey));

    return l_rootJson;
}