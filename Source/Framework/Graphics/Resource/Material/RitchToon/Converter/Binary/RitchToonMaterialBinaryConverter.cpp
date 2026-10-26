#include "RitchToonMaterialBinaryConverter.h"

std::uint64_t FWK::Converter::RitchToonMaterialBinaryConverter::CalculatePayloadSize(const Graphics::RitchToonMaterial& a_ritchToonMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter) const
{
    const auto& l_baseColorTextureFilePath    = a_ritchToonMaterial.GetREFBaseColorTextureFilePath   ();
    const auto& l_normalTextureFilePath       = a_ritchToonMaterial.GetREFNormalTextureFilePath      ();
    const auto& l_shadowMaskTextureFilePath   = a_ritchToonMaterial.GetREFShadowMaskTextureFilePath  ();
    const auto& l_matcapTextureFilePath       = a_ritchToonMaterial.GetREFMatcapTextureFilePath      ();
    const auto& l_matcapMaskTextureFilePath   = a_ritchToonMaterial.GetREFMatcapMaskTextureFilePath  ();
    const auto& l_metallicMaskTextureFilePath = a_ritchToonMaterial.GetREFMetallicMaskTextureFilePath();
    const auto& l_emissionMaskTextureFilePath = a_ritchToonMaterial.GetREFEmissionMaskTextureFilePath();
    const auto& l_outlineMaskTextureFilePath  = a_ritchToonMaterial.GetREFOutlineMaskTextureFilePath ();

    // 色6つ(ベースカラー、影、マットキャップ、光沢、発光、輪郭線)
    auto l_payloadSize = a_materialBaseBinaryConverter.CalculateSingleSize<TypeAlias::Math::Color>() * k_colorCount;

    // 値7つ(影の境界、影のぼかし、法線の強さ、滑らかさ、輪郭線の太さ、発光色へのメインテクスチャの色の混ぜ具合、発光で置き換える割合)
    l_payloadSize += a_materialBaseBinaryConverter.CalculateSingleSize<float>() * k_factorCount;

    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_baseColorTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_normalTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_shadowMaskTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_matcapTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_matcapMaskTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_metallicMaskTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_emissionMaskTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_outlineMaskTextureFilePath);

    return l_payloadSize;
}
void FWK::Converter::RitchToonMaterialBinaryConverter::WritePayload(const Graphics::RitchToonMaterial& a_ritchToonMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    const auto& l_baseColorFactor = a_ritchToonMaterial.GetREFBaseColorFactor();
    const auto& l_shadowColor     = a_ritchToonMaterial.GetREFShadowColor    ();
    const auto& l_matcapColor     = a_ritchToonMaterial.GetREFMatcapColor    ();
    const auto& l_specularColor   = a_ritchToonMaterial.GetREFSpecularColor  ();
    const auto& l_emissionColor   = a_ritchToonMaterial.GetREFEmissionColor  ();
    const auto& l_outlineColor    = a_ritchToonMaterial.GetREFOutlineColor   ();

    const float l_shadowBorder   = a_ritchToonMaterial.GetVALShadowBorder  ();
    const float l_shadowBlur     = a_ritchToonMaterial.GetVALShadowBlur    ();
    const float l_normalStrength = a_ritchToonMaterial.GetVALNormalStrength();
    const float l_smoothness     = a_ritchToonMaterial.GetVALSmoothness    ();
    const float l_outlineWidth   = a_ritchToonMaterial.GetVALOutlineWidth  ();

    const float l_emissionBaseColorBlend = a_ritchToonMaterial.GetVALEmissionBaseColorBlend();
    const float l_emissionReplaceRate    = a_ritchToonMaterial.GetVALEmissionReplaceRate   ();

    const auto& l_baseColorTextureFilePath    = a_ritchToonMaterial.GetREFBaseColorTextureFilePath   ();
    const auto& l_normalTextureFilePath       = a_ritchToonMaterial.GetREFNormalTextureFilePath      ();
    const auto& l_shadowMaskTextureFilePath   = a_ritchToonMaterial.GetREFShadowMaskTextureFilePath  ();
    const auto& l_matcapTextureFilePath       = a_ritchToonMaterial.GetREFMatcapTextureFilePath      ();
    const auto& l_matcapMaskTextureFilePath   = a_ritchToonMaterial.GetREFMatcapMaskTextureFilePath  ();
    const auto& l_metallicMaskTextureFilePath = a_ritchToonMaterial.GetREFMetallicMaskTextureFilePath();
    const auto& l_emissionMaskTextureFilePath = a_ritchToonMaterial.GetREFEmissionMaskTextureFilePath();
    const auto& l_outlineMaskTextureFilePath  = a_ritchToonMaterial.GetREFOutlineMaskTextureFilePath ();

    // 色 -> 値 -> テクスチャのパスの順に書き込む(読み込みも同じ順で行う)
    a_materialBaseBinaryConverter.WriteSingle(l_baseColorFactor, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_shadowColor,     a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_matcapColor,     a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_specularColor,   a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_emissionColor,   a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_outlineColor,    a_memoryWriteOffset);

    a_materialBaseBinaryConverter.WriteSingle(l_shadowBorder,   a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_shadowBlur,     a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_normalStrength, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_smoothness,     a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_outlineWidth,   a_memoryWriteOffset);

    a_materialBaseBinaryConverter.WriteSingle(l_emissionBaseColorBlend, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_emissionReplaceRate,    a_memoryWriteOffset);

    a_materialBaseBinaryConverter.WriteWString(l_baseColorTextureFilePath,    a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_normalTextureFilePath,       a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_shadowMaskTextureFilePath,   a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_matcapTextureFilePath,       a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_matcapMaskTextureFilePath,   a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_metallicMaskTextureFilePath, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_emissionMaskTextureFilePath, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_outlineMaskTextureFilePath,  a_memoryWriteOffset);
}
bool FWK::Converter::RitchToonMaterialBinaryConverter::ReadPayload(Graphics::RitchToonMaterial& a_ritchToonMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset) const
{
    TypeAlias::Math::Color l_baseColorFactor = Constant::k_defaultRitchToonMaterialBaseColorFactor;
    TypeAlias::Math::Color l_shadowColor     = Constant::k_defaultRitchToonMaterialShadowColor;
    TypeAlias::Math::Color l_matcapColor     = Constant::k_defaultRitchToonMaterialMatcapColor;
    TypeAlias::Math::Color l_specularColor   = Constant::k_defaultRitchToonMaterialSpecularColor;
    TypeAlias::Math::Color l_emissionColor   = Constant::k_defaultRitchToonMaterialEmissionColor;
    TypeAlias::Math::Color l_outlineColor    = Constant::k_defaultRitchToonMaterialOutlineColor;

    std::wstring l_baseColorTextureFilePath    = {};
    std::wstring l_normalTextureFilePath       = {};
    std::wstring l_shadowMaskTextureFilePath   = {};
    std::wstring l_matcapTextureFilePath       = {};
    std::wstring l_matcapMaskTextureFilePath   = {};
    std::wstring l_metallicMaskTextureFilePath = {};
    std::wstring l_emissionMaskTextureFilePath = {};
    std::wstring l_outlineMaskTextureFilePath  = {};

    float l_shadowBorder   = Constant::k_defaultRitchToonMaterialShadowBorder;
    float l_shadowBlur     = Constant::k_defaultRitchToonMaterialShadowBlur;
    float l_normalStrength = Constant::k_defaultRitchToonMaterialNormalStrength;
    float l_smoothness     = Constant::k_defaultRitchToonMaterialSmoothness;
    float l_outlineWidth   = Constant::k_defaultRitchToonMaterialOutlineWidth;

    float l_emissionBaseColorBlend = Constant::k_defaultRitchToonMaterialEmissionBaseColorBlend;
    float l_emissionReplaceRate    = Constant::k_defaultRitchToonMaterialEmissionReplaceRate;

    // 途中で失敗した場合にMaterialが中途半端な値にならないよう、全て読めてからMaterialへ設定する
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_baseColorFactor, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_shadowColor,     a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_matcapColor,     a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_specularColor,   a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_emissionColor,   a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_outlineColor,    a_memoryReadOffset)) { return false; }

    if (!a_materialBaseBinaryConverter.TryReadSingle(l_shadowBorder,   a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_shadowBlur,     a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_normalStrength, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_smoothness,     a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_outlineWidth,   a_memoryReadOffset)) { return false; }

    if (!a_materialBaseBinaryConverter.TryReadSingle(l_emissionBaseColorBlend, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_emissionReplaceRate,    a_memoryReadOffset)) { return false; }

    if (!a_materialBaseBinaryConverter.TryReadWString(l_baseColorTextureFilePath,    a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_normalTextureFilePath,       a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_shadowMaskTextureFilePath,   a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_matcapTextureFilePath,       a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_matcapMaskTextureFilePath,   a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_metallicMaskTextureFilePath, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_emissionMaskTextureFilePath, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_outlineMaskTextureFilePath,  a_memoryReadOffset)) { return false; }

    a_ritchToonMaterial.SetBaseColorFactor(l_baseColorFactor);
    a_ritchToonMaterial.SetShadowColor    (l_shadowColor);
    a_ritchToonMaterial.SetMatcapColor    (l_matcapColor);
    a_ritchToonMaterial.SetSpecularColor  (l_specularColor);
    a_ritchToonMaterial.SetEmissionColor  (l_emissionColor);
    a_ritchToonMaterial.SetOutlineColor   (l_outlineColor);

    a_ritchToonMaterial.SetShadowBorder  (l_shadowBorder);
    a_ritchToonMaterial.SetShadowBlur    (l_shadowBlur);
    a_ritchToonMaterial.SetNormalStrength(l_normalStrength);
    a_ritchToonMaterial.SetSmoothness    (l_smoothness);
    a_ritchToonMaterial.SetOutlineWidth  (l_outlineWidth);

    a_ritchToonMaterial.SetEmissionBaseColorBlend(l_emissionBaseColorBlend);
    a_ritchToonMaterial.SetEmissionReplaceRate   (l_emissionReplaceRate);

    a_ritchToonMaterial.SetBaseColorTextureFilePath   (l_baseColorTextureFilePath);
    a_ritchToonMaterial.SetNormalTextureFilePath      (l_normalTextureFilePath);
    a_ritchToonMaterial.SetShadowMaskTextureFilePath  (l_shadowMaskTextureFilePath);
    a_ritchToonMaterial.SetMatcapTextureFilePath      (l_matcapTextureFilePath);
    a_ritchToonMaterial.SetMatcapMaskTextureFilePath  (l_matcapMaskTextureFilePath);
    a_ritchToonMaterial.SetMetallicMaskTextureFilePath(l_metallicMaskTextureFilePath);
    a_ritchToonMaterial.SetEmissionMaskTextureFilePath(l_emissionMaskTextureFilePath);
    a_ritchToonMaterial.SetOutlineMaskTextureFilePath (l_outlineMaskTextureFilePath);

    return true;
}