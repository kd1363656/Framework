#include "DefaultTexture.h"

void FWK::Graphics::DefaultTexture::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::DefaultTexture::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::DefaultTexture::CreateTextureBatchUploadRecord(const Device&                           a_device, 
                                                                   const GPUMemoryAllocator&               a_gpuMemoryAllocator, 
                                                                   const TextureBatchUploadRecordBuilder&  a_textureBatchUploadRecordBuilder, 
                                                                   const TypeAlias::StorageID              a_storageID, 
                                                                         TypeAlias::SRVDescriptorPool&     a_srvDescriptorPool, 
                                                                         Struct::TextureBatchUploadRecord& a_textureBatchUploadRecord) const
{
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_textureName.empty(),                       "DefaultTextureの名前が空のため、DefaultTextureの作成処理に失敗しました。",      false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_format == DXGI_FORMAT_UNKNOWN,             "DefaultTextureのFormatが無効のため、DefaultTextureの作成処理に失敗しました。",  false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_storageID == Constant::k_invalidStorageID, "DefaultTexture用StorageIDが無効のため、DefaultTexture作成処理に失敗しました。", false);

    DirectX::ScratchImage l_scratchImage = {};

    // Jsonから読み込んだ色をもとに、1x1の画像データをCPU側に作成する
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateScratchImage(l_scratchImage), "DefaultTexture用ScratchImage作成に失敗しました。", false);

    const auto& l_texMetadata = l_scratchImage.GetMetadata();

    // ScratchImageからGPUTextureResource,UploadBuffer、SRVを作る
    // ここではまだCopyCommandQueueへ送らず、TextureSystemのPendingMapへ登録するためのRecordを作るだけ
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_textureBatchUploadRecordBuilder.CreateTextureBatchUploadRecord(l_scratchImage,
                                                                                                        l_texMetadata,
                                                                                                        a_device,
                                                                                                        a_gpuMemoryAllocator,
                                                                                                        m_textureName,
                                                                                                        a_storageID,
                                                                                                        a_srvDescriptorPool,
                                                                                                        a_textureBatchUploadRecord),
                                                                                                        "DefaultTexture用TextureBatchUploadRecord作成に失敗しました。",
                                                                                                        false);

    return true;
}

void FWK::Graphics::DefaultTexture::ApplyColorChannel(const Enum::DefaultTextureColorChannel a_colorChannel, const std::uint8_t a_colorValue)
{
    const auto l_colorChannelIndex = static_cast<std::size_t>(a_colorChannel);

    FWK_ASSERT_RETURN_IF_FAILED(l_colorChannelIndex >= m_color.size(), "DefaultTextureColorChannelが範囲外です。");

    m_color[l_colorChannelIndex] = a_colorValue;
}

std::uint8_t FWK::Graphics::DefaultTexture::FetchVALColorChannel(const Enum::DefaultTextureColorChannel a_colorChannel) const
{
    const auto l_colorChannelIndex = static_cast<std::size_t>(a_colorChannel);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_colorChannelIndex >= m_color.size(), "DefaultTextureColorChannelが範囲外です。", Constant::k_maxDefaultTextureColorChannelValue);

    return m_color[l_colorChannelIndex];
}

bool FWK::Graphics::DefaultTexture::CreateScratchImage(DirectX::ScratchImage& a_scratchImage) const
{
    // DirectXTexのScratchImageに、1x1のTexture2D領域を作成する
	// ScratchImage::Initialize2D(テクスチャフォーマット,
	//		                      横幅,
	//		                      縦幅,
	//		                      配列数,
	//		                      MIP数)
    const auto l_hr = a_scratchImage.Initialize2D(m_format,
                                                  k_defaultTextureWidth,
                                                  k_defaultTextureHeight,
                                                  k_defaultTextureArraySize,
                                                  k_defaultTextureMipLevels);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "DefaultTexture用ScratchImageの初期化に失敗しました。", false);

    // 作成したScratchImageから、実際のピクセルデータを書き込むためのImageを取得する
    const auto* l_image = a_scratchImage.GetImage(k_defaultTextureMipIndex, k_defaultTextureItemIndex, k_defaultTextureSliceIndex);

    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_image,         "DefaultTexture用Imageの取得に失敗しました。",     false);
    FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_image->pixels, "DefaultTexture用Pixel領域の取得に失敗しました。", false);

    // R8G8B8A8_UNORM / R8G8B8A8_UNORM_SRGB は、
    // 1チャンネル8bit、合計4byteのRGBAピクセルとして扱える
    // m_colorはDefaultTextureColorChannel::R/G/B/Aの順番で並ぶstd::array
    // そのため、配列の先頭から4byteをそのまま1ピクセルとして書き込める
    std::memcpy(l_image->pixels, m_color.data(), m_color.size() * sizeof(DefaultTextureColor::value_type));

    return true;
}