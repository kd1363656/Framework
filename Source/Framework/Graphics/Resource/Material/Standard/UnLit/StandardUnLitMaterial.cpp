#include "StandardUnLitMaterial.h"

bool FWK::Graphics::StandardUnLitMaterial::Deserialize(const nlohmann::json& a_rootJson)
{
    return m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::StandardUnLitMaterial::BuildRuntimeResources()
{
    // テクスチャが見つからない場合は、デフォルトのベースカラーテクスチャになる
    m_baseColorTexture.Load(FetchVALResolvedFilePath(m_baseColorTextureFilePath), Enum::TextureLoadColorSpace::SRGB, Enum::DefaultTextureType::BaseColor);
}

bool FWK::Graphics::StandardUnLitMaterial::CreateGPUDataBuffer(const Device&                                                   a_device,
                                                               const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                                     std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                     TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool)
{
    Struct::SBStandardUnLitMaterial l_gpuData = {};

    l_gpuData.m_baseColorFactor                    = m_baseColorFactor;
    l_gpuData.m_baseColorTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(m_baseColorTexture);

    return CreateGPUDataBufferFromGPUData(l_gpuData,
                                          a_device,
                                          a_gpuMemoryAllocator,
                                          a_bufferUploadCommandList,
                                          a_cbvSRVUAVDescriptorPool);
}

nlohmann::json FWK::Graphics::StandardUnLitMaterial::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::uint64_t FWK::Graphics::StandardUnLitMaterial::CalculateBinaryPayloadSize(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter) const
{
    return m_binaryConverter.CalculatePayloadSize(*this, a_materialBaseBinaryConverter);
}
void FWK::Graphics::StandardUnLitMaterial::WriteBinaryPayload(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    m_binaryConverter.WritePayload(*this, a_materialBaseBinaryConverter, a_memoryWriteOffset);
}
bool FWK::Graphics::StandardUnLitMaterial::ReadBinaryPayload(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset)
{
    return m_binaryConverter.ReadPayload(*this, a_materialBaseBinaryConverter, a_memoryReadOffset);
}

bool FWK::Graphics::StandardUnLitMaterial::EditInspectorParameter()
{
    return m_editInspector.EditInspector(*this);
}