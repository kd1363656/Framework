#include "StandardUnLitMaterialJsonConverter.h"

bool FWK::Converter::StandardUnLitMaterialJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::StandardUnLitMaterial& a_standardUnLitMaterial) const
{
    // 型名が違うjsonの値は読まない
    if (!a_standardUnLitMaterial.MaterialBase::Deserialize(a_rootJson)) { return false; }

    a_standardUnLitMaterial.SetBaseColorFactor         (Utility::DeserializeColor  (a_rootJson, k_baseColorFactorJsonKey));
    a_standardUnLitMaterial.SetBaseColorTextureFilePath(Utility::DeserializeWString(a_rootJson, k_baseColorTextureFilePathJsonKey));

    return true;
}

nlohmann::json FWK::Converter::StandardUnLitMaterialJsonConverter::Serialize(const Graphics::StandardUnLitMaterial& a_standardUnLitMaterial) const
{
    auto l_rootJson = a_standardUnLitMaterial.MaterialBase::Serialize();

    const auto& l_baseColorFactor          = a_standardUnLitMaterial.GetREFBaseColorFactor         ();
    const auto& l_baseColorTextureFilePath = a_standardUnLitMaterial.GetREFBaseColorTextureFilePath();

    Utility::UpdateJson(l_rootJson, Utility::SerializeColor  (l_baseColorFactor,          k_baseColorFactorJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_baseColorTextureFilePath, k_baseColorTextureFilePathJsonKey));

    return l_rootJson;
}