#include "RitchToonMaterialJsonConverter.h"

bool FWK::Converter::RitchToonMaterialJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RitchToonMaterial& a_ritchToonMaterial) const
{
    // 型名が違うjsonの値は読まない
    if (!a_ritchToonMaterial.MaterialBase::Deserialize(a_rootJson)) { return false; }

    a_ritchToonMaterial.SetBaseColorFactor(Utility::DeserializeColor(a_rootJson, k_baseColorFactorJsonKey));
    a_ritchToonMaterial.SetShadowColor    (Utility::DeserializeColor(a_rootJson, k_shadowColorJsonKey));
    a_ritchToonMaterial.SetMatcapColor    (Utility::DeserializeColor(a_rootJson, k_matcapColorJsonKey));
    a_ritchToonMaterial.SetSpecularColor  (Utility::DeserializeColor(a_rootJson, k_specularColorJsonKey));
    a_ritchToonMaterial.SetEmissionColor  (Utility::DeserializeColor(a_rootJson, k_emissionColorJsonKey));
    a_ritchToonMaterial.SetOutlineColor   (Utility::DeserializeColor(a_rootJson, k_outlineColorJsonKey));

    a_ritchToonMaterial.SetShadowBorder  (a_rootJson.value(k_shadowBorderJsonKey,   Constant::k_defaultRitchToonMaterialShadowBorder));
    a_ritchToonMaterial.SetShadowBlur    (a_rootJson.value(k_shadowBlurJsonKey,     Constant::k_defaultRitchToonMaterialShadowBlur));
    a_ritchToonMaterial.SetNormalStrength(a_rootJson.value(k_normalStrengthJsonKey, Constant::k_defaultRitchToonMaterialNormalStrength));
    a_ritchToonMaterial.SetSmoothness    (a_rootJson.value(k_smoothnessJsonKey,     Constant::k_defaultRitchToonMaterialSmoothness));
    a_ritchToonMaterial.SetOutlineWidth  (a_rootJson.value(k_outlineWidthJsonKey,   Constant::k_defaultRitchToonMaterialOutlineWidth));

    a_ritchToonMaterial.SetEmissionBaseColorBlend(a_rootJson.value(k_emissionBaseColorBlendJsonKey, Constant::k_defaultRitchToonMaterialEmissionBaseColorBlend));
    a_ritchToonMaterial.SetEmissionReplaceRate   (a_rootJson.value(k_emissionReplaceRateJsonKey,    Constant::k_defaultRitchToonMaterialEmissionReplaceRate));

    a_ritchToonMaterial.SetBaseColorTextureFilePath   (Utility::DeserializeWString(a_rootJson, k_baseColorTextureFilePathJsonKey));
    a_ritchToonMaterial.SetNormalTextureFilePath      (Utility::DeserializeWString(a_rootJson, k_normalTextureFilePathJsonKey));
    a_ritchToonMaterial.SetShadowMaskTextureFilePath  (Utility::DeserializeWString(a_rootJson, k_shadowMaskTextureFilePathJsonKey));
    a_ritchToonMaterial.SetMatcapTextureFilePath      (Utility::DeserializeWString(a_rootJson, k_matcapTextureFilePathJsonKey));
    a_ritchToonMaterial.SetMatcapMaskTextureFilePath  (Utility::DeserializeWString(a_rootJson, k_matcapMaskTextureFilePathJsonKey));
    a_ritchToonMaterial.SetMetallicMaskTextureFilePath(Utility::DeserializeWString(a_rootJson, k_metallicMaskTextureFilePathJsonKey));
    a_ritchToonMaterial.SetEmissionMaskTextureFilePath(Utility::DeserializeWString(a_rootJson, k_emissionMaskTextureFilePathJsonKey));
    a_ritchToonMaterial.SetOutlineMaskTextureFilePath (Utility::DeserializeWString(a_rootJson, k_outlineMaskTextureFilePathJsonKey));

    return true;
}

nlohmann::json FWK::Converter::RitchToonMaterialJsonConverter::Serialize(const Graphics::RitchToonMaterial& a_ritchToonMaterial) const
{
    auto l_rootJson = a_ritchToonMaterial.MaterialBase::Serialize();

    const auto& l_baseColorFactor = a_ritchToonMaterial.GetREFBaseColorFactor();
    const auto& l_shadowColor     = a_ritchToonMaterial.GetREFShadowColor    ();
    const auto& l_matcapColor     = a_ritchToonMaterial.GetREFMatcapColor    ();
    const auto& l_specularColor   = a_ritchToonMaterial.GetREFSpecularColor  ();
    const auto& l_emissionColor   = a_ritchToonMaterial.GetREFEmissionColor  ();
    const auto& l_outlineColor    = a_ritchToonMaterial.GetREFOutlineColor   ();

    const auto& l_baseColorTextureFilePath    = a_ritchToonMaterial.GetREFBaseColorTextureFilePath   ();
    const auto& l_normalTextureFilePath       = a_ritchToonMaterial.GetREFNormalTextureFilePath      ();
    const auto& l_shadowMaskTextureFilePath   = a_ritchToonMaterial.GetREFShadowMaskTextureFilePath  ();
    const auto& l_matcapTextureFilePath       = a_ritchToonMaterial.GetREFMatcapTextureFilePath      ();
    const auto& l_matcapMaskTextureFilePath   = a_ritchToonMaterial.GetREFMatcapMaskTextureFilePath  ();
    const auto& l_metallicMaskTextureFilePath = a_ritchToonMaterial.GetREFMetallicMaskTextureFilePath();
    const auto& l_emissionMaskTextureFilePath = a_ritchToonMaterial.GetREFEmissionMaskTextureFilePath();
    const auto& l_outlineMaskTextureFilePath  = a_ritchToonMaterial.GetREFOutlineMaskTextureFilePath ();

    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_baseColorFactor, k_baseColorFactorJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_shadowColor,     k_shadowColorJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_matcapColor,     k_matcapColorJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_specularColor,   k_specularColorJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_emissionColor,   k_emissionColorJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeColor(l_outlineColor,    k_outlineColorJsonKey));

    l_rootJson[k_shadowBorderJsonKey]   = a_ritchToonMaterial.GetVALShadowBorder  ();
    l_rootJson[k_shadowBlurJsonKey]     = a_ritchToonMaterial.GetVALShadowBlur    ();
    l_rootJson[k_normalStrengthJsonKey] = a_ritchToonMaterial.GetVALNormalStrength();
    l_rootJson[k_smoothnessJsonKey]     = a_ritchToonMaterial.GetVALSmoothness    ();
    l_rootJson[k_outlineWidthJsonKey]   = a_ritchToonMaterial.GetVALOutlineWidth  ();

    l_rootJson[k_emissionBaseColorBlendJsonKey] = a_ritchToonMaterial.GetVALEmissionBaseColorBlend();
    l_rootJson[k_emissionReplaceRateJsonKey]    = a_ritchToonMaterial.GetVALEmissionReplaceRate   ();

    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_baseColorTextureFilePath,    k_baseColorTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_normalTextureFilePath,       k_normalTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_shadowMaskTextureFilePath,   k_shadowMaskTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_matcapTextureFilePath,       k_matcapTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_matcapMaskTextureFilePath,   k_matcapMaskTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_metallicMaskTextureFilePath, k_metallicMaskTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_emissionMaskTextureFilePath, k_emissionMaskTextureFilePathJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeWString(l_outlineMaskTextureFilePath,  k_outlineMaskTextureFilePathJsonKey));

    return l_rootJson;
}