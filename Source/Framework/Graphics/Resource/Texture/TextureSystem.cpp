#include "TextureSystem.h"

void FWK::Graphics::TextureSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::TextureSystem::Create(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_textureStorage.Create(), "AssetStorageの作成に失敗したため、TextureSystemの作成処理に失敗しました。", false);

	// 起動時にまとめてデフォルトテクスチャの作成予約を行う
	FWK_ASSERT_RETURN_VALUE_IF(!CreateDefaultTexturesForBatchUpload(a_device, a_gpuMemoryAllocator, a_cbvSRVUAVDescriptorPool), "デフォルトテクスチャの作成処理に失敗したため、TextureSystemの作成処理に失敗しました。", false);
	
	return true;
}

FWK::Graphics::TextureSystem::TextureLoadResult FWK::Graphics::TextureSystem::LoadTextureForBatchUpload(const Device&					          a_device,
																					                    const GPUMemoryAllocator&		          a_gpuMemoryAllocator,
																					                    const std::filesystem::path&		      a_filePath, 
																					                    const Enum::TextureLoadColorSpace         a_textureLoadColorSpace,
																					                    const Enum::DefaultTextureType            a_defaultTextureType,
																					                 		  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	TextureLoadResult l_textureLoadResult = {};

	// 読み込めるファイルかどうかを確認
	// 読み込めるファイル出ない場合デフォルトテクスチャを返す
	if (!Utility::CanLoadFilePath(a_filePath, Converter::TextureBinaryConverter::k_lowerPNGExtension))
	{
		ApplyDefaultTextureToLoadResult(a_defaultTextureType, l_textureLoadResult);

		l_textureLoadResult.m_isLoadSuccess = false;

		return l_textureLoadResult;
	}

	// 成功したらキャッシュ内容が入っているのでreturn
	if (TryResolveCachedTextureResult(a_filePath, l_textureLoadResult)) { return l_textureLoadResult; }

	DirectX::ScratchImage l_scratchImage = {};
	DirectX::TexMetadata  l_texMetadata  = {};

	// .assetが存在していて、PNGより更新が古くなければ.assetを優先して読み込む
	if (!m_binaryConverter.LoadTextureAsset(a_filePath, l_scratchImage, l_texMetadata))
	{
		// .assetが読み込めなければテクスチャをロードする、失敗したらassert
		FWK_ASSERT_RETURN_VALUE_IF(!m_loader.LoadTextureFile(a_filePath, 
															 a_textureLoadColorSpace, 
															 l_scratchImage,
															 l_texMetadata),
															 "PNGテクスチャ読み込みに失敗したため、バッチテクスチャ登録に失敗しました。", 
															 l_textureLoadResult);
		
		// テクスチャの管理、アップロードを行うための情報を作成
		CreateAndRegisterPendingTextureForBachUpload(a_device,
													 a_gpuMemoryAllocator,
													 a_filePath,
													 l_scratchImage,
													 l_texMetadata,
													 a_cbvSRVUAVDescriptorPool,
													 l_textureLoadResult);

		// 読み込んだテクスチャのデータを保存、次回以降はバイナリーファイルで読み込めるようにする
		FWK_ASSERT_RETURN_VALUE_IF(!m_binaryConverter.SaveTextureAsset(a_filePath, l_scratchImage), "TextureAssetの保存に失敗しました。", l_textureLoadResult);

		return l_textureLoadResult;
	}

	// テクスチャの管理、アップロードを行うための情報を作成
	CreateAndRegisterPendingTextureForBachUpload(a_device,
												 a_gpuMemoryAllocator,
												 a_filePath,
												 l_scratchImage,
												 l_texMetadata,
												 a_cbvSRVUAVDescriptorPool,
											     l_textureLoadResult);

	return l_textureLoadResult;
}

nlohmann::json FWK::Graphics::TextureSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::TextureSystem::RegisterPendingTextures()
{
	for (const auto& [l_filePath, l_pendingTextureBatchUploadRecord] : m_pendingTextureBatchUploadRecordMap)
	{
		auto& l_textureRecord = l_pendingTextureBatchUploadRecord.m_textureRecord;

		FWK_ASSERT_RETURN_IF(!l_textureRecord,                                              "TextureRecordが無効のため、バッチテクスチャ登録に失敗しました。");
		FWK_ASSERT_RETURN_IF(!m_textureStorage.RegisterRecord(l_textureRecord, l_filePath), "TextureRecordの登録に失敗したため、バッチテクスチャ登録に失敗しました。");
	}

	// そのフレーム内でロードすべきテクスチャをすべてロードし終えた状態なのでクリア
	m_pendingTextureBatchUploadRecordMap.clear();
}

bool FWK::Graphics::TextureSystem::AddTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_textureStorage.AddReferenceCount(a_textureRecord), "AssetStorageでの参照数加算に失敗したため、テクスチャ参照数加算に失敗しました。", false);

	return true;
}
bool FWK::Graphics::TextureSystem::SubtractTextureReferenceCount(const std::weak_ptr<Graphics::TextureRecord>& a_textureRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_textureStorage.SubtractReferenceCount(a_textureRecord, a_directCommandQueue, a_resourceReleaseContext), "AssetStorageでの参照数減算に失敗したため、テクスチャ参照数減算に失敗しました。", false);

	return true;
}

void FWK::Graphics::TextureSystem::ApplyDefaultTexture(const Enum::DefaultTextureType a_defaultTextureType, const std::shared_ptr<DefaultTexture>& a_defaultTexture)
{
	const auto l_index = static_cast<std::size_t>(a_defaultTextureType);

	FWK_ASSERT_RETURN_IF(!a_defaultTexture, "インスタンス化されておらず、無効なデフォルトテクスチャです。デフォルトテクスチャの反映に失敗しました。");

	// デフォルトテクスチャのイーナムの種類を超えていないかどうかを確認
	FWK_ASSERT_RETURN_IF(l_index >= k_defaultTextureTypeCount,   "Enumの管理範囲を超えている値となっており、デフォルトテクスチャの反映に失敗しました。");
	FWK_ASSERT_RETURN_IF(l_index >= m_defaultTextureList.size(), "要素数の管理範囲を超えている値となっており、デフォルトテクスチャの反映に失敗しました。");

	m_defaultTextureList[l_index] = a_defaultTexture;
}

std::weak_ptr<FWK::Graphics::TextureRecord> FWK::Graphics::TextureSystem::FetchVALDefaultTextureRecord(const Enum::DefaultTextureType a_defaultTextureType) const
{
	const auto l_defaultTextureRecordIndex = static_cast<std::size_t>(a_defaultTextureType);

	FWK_ASSERT_RETURN_VALUE_IF(l_defaultTextureRecordIndex >= m_defaultTextureList.size(), "DefaultTextureListの範囲外となっており、TextureRecordの取得に失敗しました。", {});

	const auto& l_defaultTexture = m_defaultTextureList[l_defaultTextureRecordIndex];

	FWK_ASSERT_RETURN_VALUE_IF(!l_defaultTexture, "DefaultTextureが無効になっており、TextureRecordの取得に失敗しました。", {});

	return l_defaultTexture->GetREFTextureRecord();
}

bool FWK::Graphics::TextureSystem::CreateDefaultTexturesForBatchUpload(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	for (const auto& l_defaultTexture : m_defaultTextureList)
	{
		// Jsonに設定されていないDefaultTextureTypeはnullptrになるためスキップする
		if (!l_defaultTexture) { continue; }

		const auto& l_textureName = l_defaultTexture->GetREFTextureName();

		// DefaultTextureとして登録されているのに名前が空なのは設定ミスなので失敗扱いする
		FWK_ASSERT_RETURN_VALUE_IF(l_textureName.empty(), "DefaultTextureの名前が空のため、DefaultTextureの作成予約に失敗しました。", false);

		// すでにTextureStorageへ正式登録済みなら、同じDefaultTextureを作り直す必要はない
		if (!m_textureStorage.FindVALRecord(l_textureName).expired()) { continue; }

		// TextureStorageへ登録するためのStorageIDを先に確保する
		// 作成に失敗した場合は、このStorageIDを返却する
		const auto l_allocatedStorageID = m_textureStorage.AllocateStorageID();

		FWK_ASSERT_RETURN_VALUE_IF(l_allocatedStorageID == Constant::k_invalidStorageID, "DefaultTexture用StorageIDの割り当てに失敗しました。", false);

		TextureBatchUploadRecordBuilder::TextureBatchUploadRecord l_textureBatchUploadRecord = {};

		// DefaultTextureから1x1のScratchImageを作成し、GPUTextureResource/UploadBuffer/SRVDescriptorをまとめたBatchUploadRecordを作る
		if (!l_defaultTexture->CreateTextureBatchUploadRecord(a_device, 
															  a_gpuMemoryAllocator, 
															  m_batchUploadRecordBuilder,
															  l_allocatedStorageID,
															  a_cbvSRVUAVDescriptorPool,	
															  l_textureBatchUploadRecord))
		{
			// 作成失敗時はStorageIDを使わないので返却する
			m_textureStorage.ReleaseStorageID(l_allocatedStorageID);
			FWK_ASSERT_RETURN_VALUE          ("DefaultTexture用TextureBatchUploadRecord作成に失敗しました。", false);
		}

		// UploadSystemでまとめてDEFAULTヒープへコピーするため、PendingMapへ登録する
		// この時点ではまだTextureStorageへ正式登録しない
		m_pendingTextureBatchUploadRecordMap.try_emplace(l_textureName, std::move(l_textureBatchUploadRecord));
	}

	return true;
}

void FWK::Graphics::TextureSystem::CreateAndRegisterPendingTextureForBachUpload(const Device&				              a_device, 
																				const GPUMemoryAllocator&	              a_gpuMemoryAllocator,
																				const std::filesystem::path&              a_filePath, 
																				const DirectX::ScratchImage&              a_scratchImage, 
																				const DirectX::TexMetadata&               a_texMetadata,
																					  TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
																					  TextureLoadResult&                  a_textureLoadResult)
{

	TextureBatchUploadRecordBuilder::TextureBatchUploadRecord l_textureBatchUploadRecord = {};

	const auto l_allocatedStorageID = m_textureStorage.AllocateStorageID();

	FWK_ASSERT_RETURN_IF(l_allocatedStorageID == Constant::k_invalidStorageID, "StorageIDの割り当てに失敗したため、バッチテクスチャ登録に失敗しました。");

	// テクスチャを作成、管理するのに必要な情報すべてを作成(SRVDescriptorIndexなど)
	if (!m_batchUploadRecordBuilder.CreateTextureBatchUploadRecord(a_device,
																   a_gpuMemoryAllocator,
																   a_filePath,
																   a_scratchImage,
																   a_texMetadata,
																   l_allocatedStorageID,
																   a_cbvSRVUAVDescriptorPool,
																   l_textureBatchUploadRecord))
	{
		// テクスチャのアップロード処理に失敗したなら、StorageIDを解放しておく
		m_textureStorage.ReleaseStorageID(l_allocatedStorageID);

		FWK_ASSERT_RETURN("テクスチャアップロード情報の作成に失敗したため、バッチテクスチャ登録に失敗しました。");
	}
	
	const auto& l_textureRecord = l_textureBatchUploadRecord.m_textureRecord;

	if (!l_textureRecord)
	{
		// Allocate済みのStorageIDなので、失敗時は返却しておく
		m_textureStorage.ReleaseStorageID(l_allocatedStorageID);

		FWK_ASSERT_RETURN("TextureRecordが無効のため、バッチテクスチャ登録に失敗しました。");
	}

	a_textureLoadResult.m_storageID     = l_textureRecord->GetVALStorageID();
	a_textureLoadResult.m_textureRecord = l_textureRecord;
	a_textureLoadResult.m_isLoadSuccess = true;

	// 作成し終えたTextureBatchUploadRecordをリストに格納する
	m_pendingTextureBatchUploadRecordMap.try_emplace(a_filePath, std::move(l_textureBatchUploadRecord));
}

bool FWK::Graphics::TextureSystem::TryResolveCachedTextureResult(const std::filesystem::path& a_filePath, TextureLoadResult& a_textureLoadResult)
{
	const auto& l_filePath = a_filePath.wstring();

	// 既に登録済みのテクスチャなら再度ロード申請する必要がないのでreturn
	if (const auto& l_record = m_textureStorage.FindVALRecord(l_filePath).lock())
	{
		// 参照数を加算
		FWK_ASSERT_RETURN_VALUE_IF(!AddTextureReferenceCount(l_record), "登録済みテクスチャの参照数加算に失敗したため、バッチテクスチャ登録に失敗しました。", false);

		a_textureLoadResult.m_storageID     = l_record->GetVALStorageID();
		a_textureLoadResult.m_textureRecord = l_record;
		a_textureLoadResult.m_isLoadSuccess = true;

		return true;
	}

	// 現在のフレームで登録しようとしているパスが既に登録されているなら登録する必要がないためreturn
	if (const auto& l_itr = m_pendingTextureBatchUploadRecordMap.find(l_filePath);
		l_itr != m_pendingTextureBatchUploadRecordMap.end())
	{
		const auto& l_textureRecord = l_itr->second.m_textureRecord;

		FWK_ASSERT_RETURN_VALUE_IF(!l_textureRecord, "該当するStorageIDのテクスチャーレコードが無効のため、バッチテクスチャ登録に失敗しました。", false);

		// すでに登録予約済みのテクスチャが再度登録されたら参照カウントを増やす
		l_textureRecord->AddReferenceCount();

		a_textureLoadResult.m_storageID     = l_textureRecord->GetVALStorageID();
		a_textureLoadResult.m_textureRecord = l_textureRecord;
		a_textureLoadResult.m_isLoadSuccess = true;

		return true;
	}

	return false;
}

void FWK::Graphics::TextureSystem::ApplyDefaultTextureToLoadResult(const Enum::DefaultTextureType a_defaultTextureType, TextureLoadResult& a_textureLoadResult) const
{
	const auto& l_defaultRecordTextureWeak = FetchVALDefaultTextureRecord   (a_defaultTextureType);
	const auto& l_defaultRecordTexture     = l_defaultRecordTextureWeak.lock	();

	FWK_ASSERT_RETURN_IF(!l_defaultRecordTexture, "デフォルトテクスチャが作成されていなかったため、バッチテクスチャ登録に失敗しました。");

	// デフォルトテクスチャは消す必要がないため参照数の管理は気にしなくてもよいが
	// 一応参照数を加算しておく
	l_defaultRecordTexture->AddReferenceCount();

	a_textureLoadResult.m_storageID     = l_defaultRecordTexture->GetVALStorageID();
	a_textureLoadResult.m_textureRecord = l_defaultRecordTextureWeak;

	// テクスチャをロードできずデフォルトテクスチャをコピーしているため
	a_textureLoadResult.m_isLoadSuccess = false;
}