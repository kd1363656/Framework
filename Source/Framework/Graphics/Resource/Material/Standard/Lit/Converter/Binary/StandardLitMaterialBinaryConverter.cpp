#include "StandardLitMaterialBinaryConverter.h"

std::uint64_t FWK::Converter::StandardLitMaterialBinaryConverter::CalculatePayloadSize(const Graphics::StandardLitMaterial& a_standardLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter) const
{
    const auto& l_baseColorTextureFilePath = a_standardLitMaterial.GetREFBaseColorTextureFilePath();
    const auto& l_normalTextureFilePath    = a_standardLitMaterial.GetREFNormalTextureFilePath   ();
    const auto& l_roughnessTextureFilePath = a_standardLitMaterial.GetREFRoughnessTextureFilePath();
    const auto& l_metallicTextureFilePath  = a_standardLitMaterial.GetREFMetallicTextureFilePath ();

    auto l_payloadSize = a_materialBaseBinaryConverter.CalculateSingleSize<TypeAlias::Math::Color>();

    l_payloadSize += a_materialBaseBinaryConverter.CalculateSingleSize<float>();
    l_payloadSize += a_materialBaseBinaryConverter.CalculateSingleSize<float>();

    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_baseColorTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_normalTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_roughnessTextureFilePath);
    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_metallicTextureFilePath);

    return l_payloadSize;
}
void FWK::Converter::StandardLitMaterialBinaryConverter::WritePayload(const Graphics::StandardLitMaterial& a_standardLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    const auto& l_baseColorFactor          = a_standardLitMaterial.GetREFBaseColorFactor         ();
    const auto& l_baseColorTextureFilePath = a_standardLitMaterial.GetREFBaseColorTextureFilePath();
    const auto& l_normalTextureFilePath    = a_standardLitMaterial.GetREFNormalTextureFilePath   ();
    const auto& l_roughnessTextureFilePath = a_standardLitMaterial.GetREFRoughnessTextureFilePath();
    const auto& l_metallicTextureFilePath  = a_standardLitMaterial.GetREFMetallicTextureFilePath ();

    const float l_roughnessFactor = a_standardLitMaterial.GetVALRoughnessFactor();
    const float l_metallicFactor  = a_standardLitMaterial.GetVALMetallicFactor ();

    // 係数 -> テクスチャのパスの順に書き込む(読み込みも同じ順で行う)
    a_materialBaseBinaryConverter.WriteSingle(l_baseColorFactor, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_roughnessFactor, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteSingle(l_metallicFactor,  a_memoryWriteOffset);

    a_materialBaseBinaryConverter.WriteWString(l_baseColorTextureFilePath, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_normalTextureFilePath,    a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_roughnessTextureFilePath, a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_metallicTextureFilePath,  a_memoryWriteOffset);
}
bool FWK::Converter::StandardLitMaterialBinaryConverter::ReadPayload(Graphics::StandardLitMaterial& a_standardLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset) const
{
    TypeAlias::Math::Color l_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

    std::wstring l_baseColorTextureFilePath = {};
    std::wstring l_normalTextureFilePath    = {};
    std::wstring l_roughnessTextureFilePath = {};
    std::wstring l_metallicTextureFilePath  = {};

    float l_roughnessFactor = Constant::k_defaultStandardMaterialRoughnessFactor;
    float l_metallicFactor  = Constant::k_defaultStandardMaterialMetallicFactor;

    // 途中で失敗した場合にMaterialが中途半端な値にならないよう、全て読めてからMaterialへ設定する
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_baseColorFactor, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_roughnessFactor, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadSingle(l_metallicFactor,  a_memoryReadOffset)) { return false; }

    if (!a_materialBaseBinaryConverter.TryReadWString(l_baseColorTextureFilePath, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_normalTextureFilePath,    a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_roughnessTextureFilePath, a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_metallicTextureFilePath,  a_memoryReadOffset)) { return false; }

    a_standardLitMaterial.SetBaseColorFactor(l_baseColorFactor);
    a_standardLitMaterial.SetRoughnessFactor(l_roughnessFactor);
    a_standardLitMaterial.SetMetallicFactor (l_metallicFactor);

    a_standardLitMaterial.SetBaseColorTextureFilePath(l_baseColorTextureFilePath);
    a_standardLitMaterial.SetNormalTextureFilePath   (l_normalTextureFilePath);
    a_standardLitMaterial.SetRoughnessTextureFilePath(l_roughnessTextureFilePath);
    a_standardLitMaterial.SetMetallicTextureFilePath (l_metallicTextureFilePath);

    return true;
}