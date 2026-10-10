#include "ModelMaterialBase.h"

FWK::Graphics::ModelMaterialBase::ModelMaterialBase() :
    m_table(),

    m_baseColorTexture       (),
    m_baseColorRuntimeTexture(),

    m_binaryConverter(),

    m_baseColor(Constant::k_whiteColor),

    m_tableElementIndex(GPUElementTable::k_invalidElementIndex)
{
    m_baseColorTexture.SetAllowedType(Enum::AssetFilePathType::Texture);
}
FWK::Graphics::ModelMaterialBase::~ModelMaterialBase()
{
    ReleaseGPUData();
}

bool FWK::Graphics::ModelMaterialBase::Load(const std::filesystem::path& a_filePath)
{
    return false;
}

bool FWK::Graphics::ModelMaterialBase::Save(const std::filesystem::path& a_filePath)
{
    return false;
}

bool FWK::Graphics::ModelMaterialBase::TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset)
{
    return false;
}
void FWK::Graphics::ModelMaterialBase::WriteBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const
{}

std::uint64_t FWK::Graphics::ModelMaterialBase::CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const
{
    return 0ULL;
}

void FWK::Graphics::ModelMaterialBase::LoadRuntimeTextures()
{}

bool FWK::Graphics::ModelMaterialBase::CreateGPUData()
{
    return false;
}

void FWK::Graphics::ModelMaterialBase::ApplyGPUData() const
{}

void FWK::Graphics::ModelMaterialBase::LoadRuntimeTexture(const AssetFilePath&              a_assetFilePath,
                                                          const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
                                                          const Enum::DefaultTextureType    a_defaultTextureType,
                                                                Texture&                    a_runtimeTexture) const
{}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ModelMaterialBase::FetchVALTextureSRVDescriptorIndex(const Texture& a_texture) const
{
    return DescriptorHeap::k_invalidDescriptorIndex;
}

void FWK::Graphics::ModelMaterialBase::ReleaseGPUData()
{}