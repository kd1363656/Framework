#include "StandardLitMaterial.h"

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

std::uint64_t FWK::Graphics::StandardLitMaterial::CalculateBinaryPayloadSize(const Converter::MaterialBinaryConverter& a_materialBinaryConverter) const
{
    auto l_payloadSize = a_materialBinaryConverter.CalculateSingleSize<TypeAlias::Math::Color>();

    l_payloadSize += a_materialBinaryConverter.CalculateSingleSize<float>();
    l_payloadSize += a_materialBinaryConverter.CalculateSingleSize<float>();

    l_payloadSize += a_materialBinaryConverter.CalculateWStringSize(m_baseColorTextureFilePath);
    l_payloadSize += a_materialBinaryConverter.CalculateWStringSize(m_normalTextureFilePath);
    l_payloadSize += a_materialBinaryConverter.CalculateWStringSize(m_roughnessTextureFilePath);
    l_payloadSize += a_materialBinaryConverter.CalculateWStringSize(m_metallicTextureFilePath);

    return l_payloadSize;
}
void FWK::Graphics::StandardLitMaterial::WriteBinaryPayload(const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    a_materialBinaryConverter.WriteSingle(m_baseColorFactor, a_memoryWriteOffset);
    a_materialBinaryConverter.WriteSingle(m_roughnessFactor, a_memoryWriteOffset);
    a_materialBinaryConverter.WriteSingle(m_metallicFactor,  a_memoryWriteOffset);

    a_materialBinaryConverter.WriteWString(m_baseColorTextureFilePath, a_memoryWriteOffset);
    a_materialBinaryConverter.WriteWString(m_normalTextureFilePath,    a_memoryWriteOffset);
    a_materialBinaryConverter.WriteWString(m_roughnessTextureFilePath, a_memoryWriteOffset);
    a_materialBinaryConverter.WriteWString(m_metallicTextureFilePath,  a_memoryWriteOffset);
}
bool FWK::Graphics::StandardLitMaterial::ReadBinaryPayload(const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryReadOffset)
{
    if (!a_materialBinaryConverter.TryReadSingle(m_baseColorFactor, a_memoryReadOffset)) { return false; }
    if (!a_materialBinaryConverter.TryReadSingle(m_roughnessFactor, a_memoryReadOffset)) { return false; }
    if (!a_materialBinaryConverter.TryReadSingle(m_metallicFactor,  a_memoryReadOffset)) { return false; }

    if (!a_materialBinaryConverter.TryReadWString(m_baseColorTextureFilePath, a_memoryReadOffset)) { return false; }
    if (!a_materialBinaryConverter.TryReadWString(m_normalTextureFilePath,    a_memoryReadOffset)) { return false; }
    if (!a_materialBinaryConverter.TryReadWString(m_roughnessTextureFilePath, a_memoryReadOffset)) { return false; }
    if (!a_materialBinaryConverter.TryReadWString(m_metallicTextureFilePath,  a_memoryReadOffset)) { return false; }

    return true;
}

bool FWK::Graphics::StandardLitMaterial::EditInspectorParameter()
{
    return m_inspector.EditInspector(*this);
}