#include "RitchToonMaterial.h"

bool FWK::Graphics::RitchToonMaterial::Deserialize(const nlohmann::json& a_rootJson)
{
    return m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Graphics::RitchToonMaterial::BuildRuntimeResources()
{
    // テクスチャが見つからない場合は、種類ごとのデフォルトテクスチャになる
    // ・マスク     : 白(マスク無し = 全体に効く)
    // ・マットキャップ : 黒(加算しても色が変わらない)
    // メインテクスチャとマットキャップは「見た目の色」なのでSRGB、それ以外は「値」なのでLinearで読む
    m_baseColorTexture.Load   (FetchVALResolvedFilePath(m_baseColorTextureFilePath),    Enum::TextureLoadColorSpace::SRGB,   Enum::DefaultTextureType::BaseColor);
    m_normalTexture.Load      (FetchVALResolvedFilePath(m_normalTextureFilePath),       Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Normal);
    m_shadowMaskTexture.Load  (FetchVALResolvedFilePath(m_shadowMaskTextureFilePath),   Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Mask);
    m_matcapTexture.Load      (FetchVALResolvedFilePath(m_matcapTextureFilePath),       Enum::TextureLoadColorSpace::SRGB,   Enum::DefaultTextureType::Matcap);
    m_matcapMaskTexture.Load  (FetchVALResolvedFilePath(m_matcapMaskTextureFilePath),   Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Mask);
    m_metallicMaskTexture.Load(FetchVALResolvedFilePath(m_metallicMaskTextureFilePath), Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Mask);
    m_emissionMaskTexture.Load(FetchVALResolvedFilePath(m_emissionMaskTextureFilePath), Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Mask);
    m_outlineMaskTexture.Load (FetchVALResolvedFilePath(m_outlineMaskTextureFilePath),  Enum::TextureLoadColorSpace::Linear, Enum::DefaultTextureType::Mask);
}

bool FWK::Graphics::RitchToonMaterial::CreateGPUDataBuffer(const Device&                                                   a_device,
                                                           const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                                                 std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                                                 TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool)
{
    Struct::SBRitchToonMaterial l_gpuData = {};

    // ベースカラーは不透明度(A)も使うためRGBAのまま、それ以外の色はRGBだけを送る
    l_gpuData.m_baseColorFactor = m_baseColorFactor;
    l_gpuData.m_shadowColor     = m_shadowColor.ToVector3  ();
    l_gpuData.m_matcapColor     = m_matcapColor.ToVector3  ();
    l_gpuData.m_specularColor   = m_specularColor.ToVector3();
    l_gpuData.m_emissionColor   = m_emissionColor.ToVector3();
    l_gpuData.m_outlineColor    = m_outlineColor.ToVector3 ();

    l_gpuData.m_shadowBorder   = m_shadowBorder;
    l_gpuData.m_shadowBlur     = m_shadowBlur;
    l_gpuData.m_normalStrength = m_normalStrength;
    l_gpuData.m_smoothness     = m_smoothness;
    l_gpuData.m_outlineWidth   = m_outlineWidth;

    l_gpuData.m_emissionBaseColorBlend = m_emissionBaseColorBlend;
    l_gpuData.m_emissionReplaceRate    = m_emissionReplaceRate;

    l_gpuData.m_baseColorTextureSRVDescriptorIndex    = FetchVALTextureSRVDescriptorIndex(m_baseColorTexture);
    l_gpuData.m_normalTextureSRVDescriptorIndex       = FetchVALTextureSRVDescriptorIndex(m_normalTexture);
    l_gpuData.m_shadowMaskTextureSRVDescriptorIndex   = FetchVALTextureSRVDescriptorIndex(m_shadowMaskTexture);
    l_gpuData.m_matcapTextureSRVDescriptorIndex       = FetchVALTextureSRVDescriptorIndex(m_matcapTexture);
    l_gpuData.m_matcapMaskTextureSRVDescriptorIndex   = FetchVALTextureSRVDescriptorIndex(m_matcapMaskTexture);
    l_gpuData.m_metallicMaskTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(m_metallicMaskTexture);
    l_gpuData.m_emissionMaskTextureSRVDescriptorIndex = FetchVALTextureSRVDescriptorIndex(m_emissionMaskTexture);
    l_gpuData.m_outlineMaskTextureSRVDescriptorIndex  = FetchVALTextureSRVDescriptorIndex(m_outlineMaskTexture);

    return CreateGPUDataBufferFromGPUData(l_gpuData,
                                          a_device,
                                          a_gpuMemoryAllocator,
                                          a_bufferUploadCommandList,
                                          a_cbvSRVUAVDescriptorPool);
}

nlohmann::json FWK::Graphics::RitchToonMaterial::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::uint64_t FWK::Graphics::RitchToonMaterial::CalculateBinaryPayloadSize(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter) const
{
    return m_binaryConverter.CalculatePayloadSize(*this, a_materialBaseBinaryConverter);
}
void FWK::Graphics::RitchToonMaterial::WriteBinaryPayload(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const
{
    m_binaryConverter.WritePayload(*this, a_materialBaseBinaryConverter, a_memoryWriteOffset);
}
bool FWK::Graphics::RitchToonMaterial::ReadBinaryPayload(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset)
{
    return m_binaryConverter.ReadPayload(*this, a_materialBaseBinaryConverter, a_memoryReadOffset);
}

bool FWK::Graphics::RitchToonMaterial::EditInspectorParameter()
{
    return m_editInspector.EditInspector(*this);
}