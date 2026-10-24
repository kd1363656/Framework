#include "StandardUnLitMaterial.h"

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

std::uint64_t FWK::Graphics::StandardUnLitMaterial::CalculateBinaryPayloadSize(const Converter::MaterialBinaryConverter& a_materialBinaryConverter) const
{
    auto l_payloadSize = a_materialBinaryConverter.CalculateSingleSize<TypeAlias::Math::Color>();

    l_payloadSize += a_materialBinaryConverter.CalculateWStringSize(m_baseColorTextureFilePath);

    return l_payloadSize;
}
void FWK::Graphics::StandardUnLitMaterial::WriteBinaryPayload(const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    a_materialBinaryConverter.WriteSingle (m_baseColorFactor,          a_memoryWriteOffset);
    a_materialBinaryConverter.WriteWString(m_baseColorTextureFilePath, a_memoryWriteOffset);
}
bool FWK::Graphics::StandardUnLitMaterial::ReadBinaryPayload(const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryReadOffset)
{
    if (!a_materialBinaryConverter.TryReadSingle (m_baseColorFactor,          a_memoryReadOffset)) { return false; }
    if (!a_materialBinaryConverter.TryReadWString(m_baseColorTextureFilePath, a_memoryReadOffset)) { return false; }

    return true;
}

bool FWK::Graphics::StandardUnLitMaterial::EditInspectorParameter()
{
    return m_inspector.EditInspector(*this);
}