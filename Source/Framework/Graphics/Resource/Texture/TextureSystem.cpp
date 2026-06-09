#include "TextureSystem.h"

void FWK::Graphics::TextureSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::TextureSystem::Create(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_textureStorage.Create(), "AssetStorageの作成に失敗したため、TextureSystemの作成処理に失敗しました。", false);

	// 起動時にまとめてデフォルトテクスチャの作成予約を行う
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!CreateDefaultTexturesForBatchUpload(a_device, a_gpuMemoryAllocator, a_srvDescriptorPool), "デフォルトテクスチャの作成処理に失敗したため、TextureSystemの作成処理に失敗しました。", false);
	
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

void FWK::Graphics::TextureSystem::RegisterPendingTextures()
{
	// もしアップロード処理が終わっていなければ
	// このMapの情報を削除、Storageに登録をしてはいけないためreturn(失敗はしていないためtrueを返す)
	if (!m_isUploadToDefaultHeapCopyCompleted) { return; }

	for (const auto& [l_filePath, l_pendingTextureBatchUploadRecord] : m_pendingTextureBatchUploadRecordMap)
	{
		auto& l_textureRecord = l_pendingTextureBatchUploadRecord.m_textureRecord;

		FWK_ASSERT_RETURN_IF_FAILED(!l_textureRecord,											   "TextureRecordが無効のため、バッチテクスチャ登録に失敗しました。");
		FWK_ASSERT_RETURN_IF_FAILED(!m_textureStorage.RegisterRecord(l_textureRecord, l_filePath), "TextureRecordの登録に失敗したため、バッチテクスチャ登録に失敗しました。");
	}

	// そのフレーム内でロードすべきテクスチャをすべてロードし終えた状態なのでクリア
	m_pendingTextureBatchUploadRecordMap.clear();

	// 次フレームのコピー完了通知が来るまでこの関数を実行しない
	m_isUploadToDefaultHeapCopyCompleted = false;
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

void FWK::Graphics::TextureSystem::ApplyDefaultTexture(const Enum::DefaultTextureType a_defaultTextureType, const std::shared_ptr<DefaultTexture>& a_defaultTexture)
{
	const auto l_index = static_cast<std::size_t>(a_defaultTextureType);

	FWK_ASSERT_RETURN_IF_FAILED(!a_defaultTexture, "インスタンス化されておらず、無効なデフォルトテクスチャです。デフォルトテクスチャの反映に失敗しました。");

	// デフォルトテクスチャのイーナムの種類を超えていないかどうかを確認
	FWK_ASSERT_RETURN_IF_FAILED(l_index >= k_defaultTextureTypeCount,   "Enumの管理範囲を超えている値となっており、デフォルトテクスチャの反映に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(l_index >= m_defaultTextureList.size(), "要素数の管理範囲を超えている値となっており、デフォルトテクスチャの反映に失敗しました。");

	m_defaultTextureList[l_index] = a_defaultTexture;
}

bool FWK::Graphics::TextureSystem::CreateDefaultTexturesForBatchUpload(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	for (const auto& l_defaultTexture : m_defaultTextureList)
	{
		// Jsonに設定されていないDefaultTextureTypeはnullptrになるためスキップする
		if (!l_defaultTexture) { continue; }

		const auto& l_textureName = l_defaultTexture->GetREFTextureName();

		// DefaultTextureとして登録されているのに名前が空なのは設定ミスなので失敗扱いする
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_textureName.empty(), "DefaultTextureの名前が空のため、DefaultTextureの作成予約に失敗しました。", false);

		// すでにTextureStorageへ正式登録済みなら、同じDefaultTextureを作り直す必要はない
		if (!m_textureStorage.FindVALRecord(l_textureName).expired()) { continue; }

		// TextureStorageへ登録するためのStorageIDを先に確保する
		// 作成に失敗した場合は、このStorageIDを返却する
		const auto l_allocatedStorageID = m_textureStorage.AllocateStorageID();

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_allocatedStorageID == Constant::k_invalidStorageID, "DefaultTexture用StorageIDの割り当てに失敗しました。", false);

		Struct::TextureBatchUploadRecord l_textureBatchUploadRecord = {};

		// DefaultTextureから1x1のScratchImageを作成し、GPUTextureResource/UploadBuffer/SRVDescriptorをまとめたBatchUploadRecordを作る
		if (!l_defaultTexture->CreateTextureBatchUploadRecord(a_device, 
															  a_gpuMemoryAllocator, 
															  m_batchUploadRecordBuilder,
															  l_allocatedStorageID,
															  a_srvDescriptorPool,	
															  l_textureBatchUploadRecord))
		{
			// 作成失敗時はStorageIDを使わないので返却する
			m_textureStorage.ReleaseStorageID(l_allocatedStorageID);

			FWK_ASSERT_RETURN_VALUE("DefaultTexture用TextureBatchUploadRecord作成に失敗しました。", false);
		}

		// UploadSystemでまとめてDEFAULTヒープへコピーするため、PendingMapへ登録する
		// この時点ではまだTextureStorageへ正式登録しない
		m_pendingTextureBatchUploadRecordMap.try_emplace(l_textureName, std::move(l_textureBatchUploadRecord));
	}

	return true;
}