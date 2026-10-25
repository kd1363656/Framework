#include "StandardLitMaterial.h"

bool FWK::Graphics::StandardLitMaterial::Deserialize(const nlohmann::json& a_rootJson)
{
    return m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::StandardLitMaterial::BuildRuntimeResources()
{
    // テクスチャが見つからない場合は、種類ごとのデフォルトテクスチャになる
    m_baseColorTexture.Load(FetchVALResolvedFilePath(m_baseColorTextureFilePath), Enum::TextureLoadColorSpace::SRGB,   Enum::DefaultTextureType::BaseColor);
    m_normalTexture.Load   (FetchVALResolvedFilePath(m_normalTextureFilePath),    Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Normal);
    m_roughnessTexture.Load(FetchVALResolvedFilePath(m_roughnessTextureFilePath), Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Roughness);
    m_metallicTexture.Load (FetchVALResolvedFilePath(m_metallicTextureFilePath),  Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Metallic);
}

bool FWK::Graphics::StandardLitMaterial::CreateGPUDataBuffer(const Device&                                                   a_device,
                                                             const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                                   std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                   TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool)
{
    Struct::SBStandardLitMaterial l_gpuData = {};

    l_gpuData.m_baseColorFactor = m_baseColorFactor;
    l_gpuData.m_roughnessFactor = m_roughnessFactor;
    l_gpuData.m_metallicFactor  = m_metallicFactor;

    l_gpuData.m_baseColorTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(m_baseColorTexture);
    l_gpuData.m_normalTextureSRVDescriptorIndex    = FetchVALTextureSRVDescriptorIndex(m_normalTexture);
    l_gpuData.m_roughnessTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(m_roughnessTexture);
    l_gpuData.m_metallicTextureSRVDescriptorIndex  = FetchVALTextureSRVDescriptorIndex(m_metallicTexture);

    return CreateGPUDataBufferFromGPUData(l_gpuData,
                                          a_device,
                                          a_gpuMemoryAllocator,
                                          a_bufferUploadCommandList,
                                          a_cbvSRVUAVDescriptorPool);
}

nlohmann::json FWK::Graphics::StandardLitMaterial::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::uint64_t FWK::Graphics::StandardLitMaterial::CalculateBinaryPayloadSize(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter) const
{
    return m_binaryConverter.CalculatePayloadSize(*this, a_materialBaseBinaryConverter);
}
void FWK::Graphics::StandardLitMaterial::WriteBinaryPayload(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    m_binaryConverter.WritePayload(*this, a_materialBaseBinaryConverter, a_memoryWriteOffset);
}
bool FWK::Graphics::StandardLitMaterial::ReadBinaryPayload(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset)
{
    return m_binaryConverter.ReadPayload(*this, a_materialBaseBinaryConverter, a_memoryReadOffset);
}

bool FWK::Graphics::StandardLitMaterial::EditInspectorParameter()
{
    return m_editInspector.EditInspector(*this);
}