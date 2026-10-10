#include "ModelStandardLitMaterial.h"

FWK::Graphics::ModelStandardLitMaterial::ModelStandardLitMaterial() :
    m_normalTexture   (),
    m_metallicTexture (),
    m_roughnessTexture(),

    m_normalRuntimeTexture   (),
    m_metallicRuntimeTexture (),
    m_roughnessRuntimeTexture(),

    m_metallic (ModelStandardLitMaterialGPUData::k_defaultMetallic),
    m_roughness(ModelStandardLitMaterialGPUData::k_defaultRoughness)
{
    m_normalTexture.SetAllowedType   (Enum::AssetFilePathType::Texture);
    m_metallicTexture.SetAllowedType (Enum::AssetFilePathType::Texture);
    m_roughnessTexture.SetAllowedType(Enum::AssetFilePathType::Texture);
}
FWK::Graphics::ModelStandardLitMaterial::~ModelStandardLitMaterial() = default;

bool FWK::Graphics::ModelStandardLitMaterial::TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset)
{
    return false;
}
void FWK::Graphics::ModelStandardLitMaterial::WriteBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const
{}

std::uint64_t FWK::Graphics::ModelStandardLitMaterial::CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const
{
    return 0ULL;
}

void FWK::Graphics::ModelStandardLitMaterial::LoadRuntimeTextures()
{}

const FWK::Struct::ModelRenderTableINFO& FWK::Graphics::ModelStandardLitMaterial::FetchREFTableINFO() const
{
    return ModelStandardLitMaterialGPUData::GetREFModelRenderTableINFO();
}

void FWK::Graphics::ModelStandardLitMaterial::WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const
{}