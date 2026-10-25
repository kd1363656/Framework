#include "StandardUnLitMaterialBinaryConverter.h"

std::uint64_t FWK::Converter::StandardUnLitMaterialBinaryConverter::CalculatePayloadSize(const Graphics::StandardUnLitMaterial& a_standardUnLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter) const
{
    const auto& l_baseColorTextureFilePath = a_standardUnLitMaterial.GetREFBaseColorTextureFilePath();

    auto l_payloadSize = a_materialBaseBinaryConverter.CalculateSingleSize<TypeAlias::Math::Color>();

    l_payloadSize += a_materialBaseBinaryConverter.CalculateWStringSize(l_baseColorTextureFilePath);

    return l_payloadSize;
}
void FWK::Converter::StandardUnLitMaterialBinaryConverter::WritePayload(const Graphics::StandardUnLitMaterial& a_standardUnLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    const auto& l_baseColorFactor          = a_standardUnLitMaterial.GetREFBaseColorFactor         ();
    const auto& l_baseColorTextureFilePath = a_standardUnLitMaterial.GetREFBaseColorTextureFilePath();

    // 係数 -> テクスチャのパスの順に書き込む(読み込みも同じ順で行う)
    a_materialBaseBinaryConverter.WriteSingle (l_baseColorFactor,          a_memoryWriteOffset);
    a_materialBaseBinaryConverter.WriteWString(l_baseColorTextureFilePath, a_memoryWriteOffset);
}
bool FWK::Converter::StandardUnLitMaterialBinaryConverter::ReadPayload(Graphics::StandardUnLitMaterial& a_standardUnLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset) const
{
    TypeAlias::Math::Color l_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

    std::wstring l_baseColorTextureFilePath = {};

    // 途中で失敗した場合にMaterialが中途半端な値にならないよう、全て読めてからMaterialへ設定する
    if (!a_materialBaseBinaryConverter.TryReadSingle (l_baseColorFactor,          a_memoryReadOffset)) { return false; }
    if (!a_materialBaseBinaryConverter.TryReadWString(l_baseColorTextureFilePath, a_memoryReadOffset)) { return false; }

    a_standardUnLitMaterial.SetBaseColorFactor         (l_baseColorFactor);
    a_standardUnLitMaterial.SetBaseColorTextureFilePath(l_baseColorTextureFilePath);

    return true;
}