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
																							 TypeAlias::SRVDescriptorPool& a_srvDescriptorPool, 
																					   const Enum::TextureLoadType		   a_loadType)
{
	Struct::TextureLoadResult l_textureLoadResult = {};

	// 読み込めるファイルかどうかを確認
	if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension)) { return l_textureLoadResult; }

	const auto& l_filePath = a_filePath.wstring();

	// 既に登録済みのテクスチャなら再度ロード申請する必要がないのでreturn
	if (const auto& l_record = m_textureStorage.FindVALRecord(l_filePath).lock())
	{
		// 参照数を加算
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!AddTextureReferenceCount(l_record), "登録済みテクスチャの参照数加算に失敗したため、バッチテクスチャ登録に失敗しました。", l_textureLoadResult);

		l_textureLoadResult.m_storageID     = l_record->GetVALStorageID();
		l_textureLoadResult.m_textureRecord = l_record;

		return l_textureLoadResult;
	}

	// 現在のフレームで登録しようとしているパスが既に登録されているなら登録する必要がないためreturn
	if (const auto& l_itr = m_pendingTextureBatchUploadRecordMap.find(l_filePath);
		l_itr != m_pendingTextureBatchUploadRecordMap.end())
	{
		const auto& l_textureRecord = l_itr->second.m_textureRecord;

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_textureRecord, "該当するStorageIDのテクスチャーレコードが無効のため、バッチテクスチャ登録に失敗しました。", l_textureLoadResult);

		// すでに登録予約済みのテクスチャが再度登録されたら参照カウントを増やす
		l_textureRecord->AddReferenceCount();

		l_textureLoadResult.m_storageID     = l_textureRecord->GetVALStorageID();
		l_textureLoadResult.m_textureRecord = l_textureRecord;

		return l_textureLoadResult;
	}

	DirectX::ScratchImage l_scratchImage = {};
	DirectX::TexMetadata  l_texMetadata  = {};

	// まずはテクスチャをロードしてする、失敗したらassert
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_loader.LoadTextureFile(a_filePath, 
																a_loadType, 
																l_scratchImage,
																l_texMetadata),
																"PNGテクスチャ読み込みに失敗したため、バッチテクスチャ登録に失敗しました。。", 
																l_textureLoadResult);

	Struct::TextureBatchUploadRecord l_textureBatchUploadRecord = {};

	const auto l_allocatedStorageID = m_textureStorage.AllocateStorageID();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_allocatedStorageID == Constant::k_invalidStorageID, "StorageIDの割り当てに失敗したため、バッチテクスチャ登録に失敗しました。", l_textureLoadResult);

	// テクスチャを作成、管理するのに必要な情報すべてを作成(SRVDescriptorIndexなど)
	if (!m_batchUploadRecordBuilder.CreateTextureBatchUploadRecord(l_scratchImage,
																   l_texMetadata,
																   a_device,
																   a_gpuMemoryAllocator,
																   a_filePath,
																   l_allocatedStorageID,
																   a_srvDescriptorPool,
																   l_textureBatchUploadRecord))
	{
		// テクスチャのアップロード処理に失敗したなら、StorageIDを解放しておく
		m_textureStorage.ReleaseStorageID(l_allocatedStorageID);

		FWK_ASSERT_RETURN_VALUE("テクスチャアップロード情報の作成に失敗したため、バッチテクスチャ登録に失敗しました。", l_textureLoadResult);
	}

	const auto& l_textureRecord = l_textureBatchUploadRecord.m_textureRecord;

	if (!l_textureRecord)
	{
		// Allocate済みのStorageIDなので、失敗時は返却しておく
		m_textureStorage.ReleaseStorageID(l_allocatedStorageID);

		FWK_ASSERT_RETURN_VALUE("TextureRecordが無効のため、バッチテクスチャ登録に失敗しました。", l_textureLoadResult);
	}

	l_textureLoadResult.m_storageID     = l_textureRecord->GetVALStorageID();
	l_textureLoadResult.m_textureRecord = l_textureRecord;

	// 作成し終えたTextureBatchUploadRecordをリストに格納する
	m_pendingTextureBatchUploadRecordMap.try_emplace(l_filePath, std::move(l_textureBatchUploadRecord));

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