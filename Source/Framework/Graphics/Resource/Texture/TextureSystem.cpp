#include "TextureSystem.h"

void FWK::Graphics::TextureSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::TextureSystem::Create()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_textureStorage.Create(), "AssetStorageの作成に失敗したため、TextureSystemの作成処理に失敗しました。", false);

	return true;
}

bool FWK::Graphics::TextureSystem::LoadTextureFile(const std::filesystem::path& a_filePath, const Enum::TextureLoadType a_loadType)
{
	DirectX::TexMetadata  l_texMetadata  = {};
	DirectX::ScratchImage l_scratchImage = {};

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_loader.LoadTextureFile(a_filePath, 
															    a_loadType, 
																l_scratchImage,
																l_texMetadata),
																"テクスチャロード処理に失敗しました。",
																false);

	return true;
}

FWK::Struct::TextureLoadResult FWK::Graphics::TextureSystem::LoadTextureForBatchUpload(const Device&					   a_device,
																					   const GPUMemoryAllocator&		   a_gpuMemoryAllocator,
																					   const std::filesystem::path&		   a_filePath, 
																							 TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	Struct::TextureLoadResult l_textureLoadResult = {};

	// 読み込めるファイルかどうかを確認
	if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension)) { return l_textureLoadResult; }

	const auto& l_filePath = a_filePath.wstring();

	// 既に登録済みのテクスチャなら再度ロード申請する必要がないのでreturn
	if (const auto& l_record = m_textureStorage.FindVALRecord(l_filePath).lock())
	{
		// 参照数を加算
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!AddTextureReferenceCount(l_record), "登録済みテクスチャの参照数加算に失敗したため、テクスチャ読み込み処理に失敗しました。", l_textureLoadResult);

		l_textureLoadResult.m_storageID     = l_record->GetVALStorageID();
		l_textureLoadResult.m_textureRecord = l_record;

		return l_textureLoadResult;
	}



	return l_textureLoadResult;
}

nlohmann::json FWK::Graphics::TextureSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::TextureSystem::AddTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_textureStorage.AddReferenceCount(a_textureRecord), "AssetStorageでの参照数加算に失敗したため、テクスチャ参照数加算に失敗しました。", false);

	return true;
}
bool FWK::Graphics::TextureSystem::SubtractTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord, const DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_textureStorage.SubtractReferenceCount(a_textureRecord, a_directCommandQueue, a_resourceReleaseContext), "AssetStorageでの参照数減算に失敗したため、テクスチャ解放予約に失敗しました。", false);

	return true;
}