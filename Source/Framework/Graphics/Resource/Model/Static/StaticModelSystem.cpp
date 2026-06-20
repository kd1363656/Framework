#include "StaticModelSystem.h"

void FWK::Graphics::StaticModelSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::StaticModelSystem::Create()
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_staticModelStorage.Create(), "AssetStorageの作成に失敗したため、StaticModelSystemの作成処理に失敗しました。", false);

	return true;
}

FWK::Struct::StaticModelLoadResult FWK::Graphics::StaticModelSystem::LoadStaticModelForBatchUpload(const Device&				           a_device,
																							       const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
																							       const std::filesystem::path&        a_filePath, 
																							    		 TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	Struct::StaticModelLoadResult l_staticModelLoadResult = {};

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "StaticModelのFBXファイルが存在しません。", l_staticModelLoadResult);

	// 成功したらキャッシュ内容が入っているのでreturn
	if (TryResolveCachedStaticModelResult(a_filePath, l_staticModelLoadResult)) { return l_staticModelLoadResult; }

	const auto l_allocateStorageID = m_staticModelStorage.AllocateStorageID();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_allocateStorageID == Constant::k_invalidStorageID, "StorageIDの割り当てに失敗したため、StaticModel読み込み処理に失敗しました。", l_staticModelLoadResult);

	auto l_staticModelRecord = std::make_shared<StaticModelRecord>();

	l_staticModelRecord->SetFilePath      (a_filePath.wstring());
	l_staticModelRecord->SetStorageID     (l_allocateStorageID);
	l_staticModelRecord->SetReferenceCount(Constant::k_defaultAssetReferenceCount);

	// .assetが存在していて、FBXより更新が古くなければ.assetを優先して読み込む
	// 失敗したらUFBXから読み込む
	if (!m_staticModelBinaryConverter.LoadStaticModelAsset(a_filePath, *l_staticModelRecord))
	{
		// .assetが読み込めなければFBXモデルをロードする、失敗したらassert
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_loader.LoadStaticModelFile(a_filePath, *l_staticModelRecord), "StaticModel読み込みに失敗したため、バッチモデル登録に失敗しました。。", {});

		// モデルのメッシュレット生成などを行う
		BuildStaticModelAssetData(a_filePath, *l_staticModelRecord);

		// 実行時に決まる情報を作成
		BuildStaticModelRuntimeData(l_staticModelRecord,
									a_device,
									a_gpuMemoryAllocator,
									l_allocateStorageID,
									a_filePath,
									a_srvDescriptorPool,
									l_staticModelLoadResult);

		return l_staticModelLoadResult;
	}

	// 実行時に決まる情報を作成
	BuildStaticModelRuntimeData(l_staticModelRecord, 
								a_device,
								a_gpuMemoryAllocator,
								l_allocateStorageID,
								a_filePath,
								a_srvDescriptorPool,
								l_staticModelLoadResult);

	return l_staticModelLoadResult;
}

nlohmann::json FWK::Graphics::StaticModelSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::StaticModelSystem::RegisterPendingStaticModels()
{
	for (const auto& [l_filePath, l_pendingStaticModelBatchUploadRecord] : m_pendingStaticModelBatchUploadRecordMap)
	{
		auto& l_staticModelRecord = l_pendingStaticModelBatchUploadRecord.m_staticModelRecord;

		FWK_ASSERT_RETURN_IF_FAILED(!l_staticModelRecord,											       "TextureRecordが無効のため、バッチスタティックモデル登録に失敗しました。");
		FWK_ASSERT_RETURN_IF_FAILED(!m_staticModelStorage.RegisterRecord(l_staticModelRecord, l_filePath), "TextureRecordの登録に失敗したため、バッチスタティックモデル登録に失敗しました。");
	}

	// そのフレーム内でロードすべきテクスチャをすべてロードし終えた状態なのでクリア
	m_pendingStaticModelBatchUploadRecordMap.clear();
}

bool FWK::Graphics::StaticModelSystem::AddStaticModelReferenceCount(const std::weak_ptr<Graphics::StaticModelRecord>& a_staticModelRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_staticModelStorage.AddReferenceCount(a_staticModelRecord), "AssetStorageでの参照数加算に失敗したため、StaticModel参照数加算に失敗しました。", false);

	return true;
}
bool FWK::Graphics::StaticModelSystem::SubtractStaticModelReferenceCount(const std::weak_ptr<Graphics::StaticModelRecord>& a_staticModelRecord, const DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_staticModelStorage.SubtractReferenceCount(a_staticModelRecord, a_directCommandQueue, a_resourceReleaseContext), "AssetStorageでの参照数減算に失敗したため、StaticModel参照数減算に失敗しました。", false);

	return true;
}

bool FWK::Graphics::StaticModelSystem::BuildStaticModelAssetData(const std::filesystem::path& a_filePath, StaticModelRecord& a_staticModelRecord)
{
	// meshoptimizerを使用して頂点とインデックスをGPUで扱いやすい配置へ最適化
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_meshOptimizer.OptimizeStaticModelRecord(a_staticModelRecord), "StaticModelMeshの最適化に失敗しました。", false);

	// MeshShaderで扱うため、最適化済みの頂点とインデックスからMeshletDataを作成
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_meshletBuilder.BuildStaticModelRecordMeshletData(a_staticModelRecord), "StaticModelMeshletDataの作成に失敗しました。", false);

	// 読み込んだFBXモデルのデータを保存、次回以降はバイナリーファイルで読み込めるようにする
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_staticModelBinaryConverter.SaveStaticModelAsset(a_filePath, a_staticModelRecord), "TextureAssetの保存に失敗しました", false);

	return true;
}
void FWK::Graphics::StaticModelSystem::BuildMaterialRuntimeTextures(const std::filesystem::path& a_filePath, StaticModelRecord& a_staticModelRecord) const
{
	// ランタイムパラメータを作成していく
	for (auto& l_modelMesh : a_staticModelRecord.GetMutableREFModelData().m_modelMeshList)
	{
		const auto& l_modelMaterialAssetData   = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
			  auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;

		// ベースカラーテクスチャの読み込み
		l_modelMaterialRuntimeData.m_baseColorTexture = CreateSingleMaterialTexture(a_filePath, 
																			        l_modelMaterialAssetData.m_baseColorTextureFilePath, 
																			        Enum::TextureLoadColorSpace::SRGB, 
																			        Enum::DefaultTextureType::BaseColor);

		// ノーマルテクスチャの読み込み
		l_modelMaterialRuntimeData.m_normalTexture = CreateSingleMaterialTexture(a_filePath, 
																		         l_modelMaterialAssetData.m_normalTextureFilePath, 
																		         Enum::TextureLoadColorSpace::Linear,
																		         Enum::DefaultTextureType::Normal);

		// メタリックテクスチャの読み込み
		l_modelMaterialRuntimeData.m_metallicTexture = CreateSingleMaterialTexture(a_filePath, 
																		           l_modelMaterialAssetData.m_normalTextureFilePath, 
																		           Enum::TextureLoadColorSpace::Linear,
																		           Enum::DefaultTextureType::Metallic);

		// ラフネステクスチャの読み込み
		l_modelMaterialRuntimeData.m_roughnessTexture = CreateSingleMaterialTexture(a_filePath, 
																		            l_modelMaterialAssetData.m_normalTextureFilePath, 
																		            Enum::TextureLoadColorSpace::Linear,
																		            Enum::DefaultTextureType::Roughness);
	}
}

void FWK::Graphics::StaticModelSystem::BuildStaticModelRuntimeData(const std::shared_ptr<StaticModelRecord>& a_staticModelRecord, 
																   const Device&			                 a_device,
																   const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
																   const TypeAlias::StorageID				 a_storageID,
																   const std::filesystem::path&				 a_filePath,
																   	     TypeAlias::SRVDescriptorPool&       a_srvDescriptorPool,
																   	     Struct::StaticModelLoadResult&      a_staticModelLoadResult)
{

	FWK_ASSERT_RETURN_IF_FAILED(!a_staticModelRecord, "StaticModelRecordが無効なため、RuntimeData構築処理処理に失敗しました。");

	Struct::StaticModelBatchUploadRecord l_staticModelBatchUploadRecord = {};

	// マテリアルで使用するテクスチャを読み込む
	BuildMaterialRuntimeTextures(a_filePath, *a_staticModelRecord);

	// バッチアップロード用情報の作成
	if (!CreateStaticBatchUploadRecord(a_staticModelRecord,
									   a_device,
									   a_gpuMemoryAllocator,
									   a_srvDescriptorPool,
									   l_staticModelBatchUploadRecord))
	{
		m_staticModelStorage.ReleaseStorageID(a_storageID);

		FWK_ASSERT_RETURN("バッチ情報の作成に失敗しており、RuntimeData構築処理処理に失敗しました。");
	}

	a_staticModelLoadResult.m_storageID			= a_staticModelRecord->GetVALStorageID();
	a_staticModelLoadResult.m_staticModelRecord = a_staticModelRecord;

	// バッチアップロード情報をマップに格納
	m_pendingStaticModelBatchUploadRecordMap.try_emplace(a_filePath.wstring(), std::move(l_staticModelBatchUploadRecord));
}

std::shared_ptr<FWK::Graphics::Texture> FWK::Graphics::StaticModelSystem::CreateSingleMaterialTexture(const std::filesystem::path&      a_modelFilePath,
																								      const std::wstring&               a_textureFilePath,
																								      const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
																								      const Enum::DefaultTextureType    a_defaultTextureType) const
{
	auto l_texture = std::make_shared<Texture>();

	std::filesystem::path l_textureFilePath = a_textureFilePath;

	// FBXから取得したTextureFilePathが相対パスの場合
	// ModelFilePathの親フォルダからの相対パスとして解決する
	if (l_textureFilePath.is_relative())
	{
		l_textureFilePath = a_modelFilePath.parent_path() / l_textureFilePath;
	}

	l_texture->Load(l_textureFilePath, a_textureLoadColorSpace, a_defaultTextureType);

	return l_texture;
}
bool FWK::Graphics::StaticModelSystem::CreateStaticBatchUploadRecord(const std::shared_ptr<StaticModelRecord>    a_staticModelRecord, 
																	 const Device&							     a_device, 
																	 const GPUMemoryAllocator&				     a_gpuMemoryAllocator, 
																	 	   TypeAlias::SRVDescriptorPool&         a_srvDescriptorPool,
																	 	   Struct::StaticModelBatchUploadRecord& a_staticModelBatchUploadRecord) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_staticModelRecord, "StaticModelRecordが無効になっており、モデルのバッチアップロードレコードの作成に失敗しました。", false);

	a_staticModelBatchUploadRecord.m_staticModelRecord = a_staticModelRecord;

	// モデルのバッチアップロード用情報の作成
	if (!m_batchUploadRecordBuilder.CreateStaticModelBatchUploadRecord(a_device,
																	   a_gpuMemoryAllocator,
																	   a_staticModelBatchUploadRecord.m_bufferUploadCommandList,
																	   a_srvDescriptorPool,
																	   *a_staticModelRecord))
	{
		FWK_ASSERT_RETURN_VALUE("StaticModel用BufferUploadCommandの作成に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::StaticModelSystem::TryResolveCachedStaticModelResult(const std::filesystem::path& a_filePath, Struct::StaticModelLoadResult& a_staticModelLoadResult)
{
	const auto& l_filePath = a_filePath.wstring();

	// 既に登録済みのStaticModelなら再度ロード申請する必要がないのでreturn
	if (const auto& l_record = m_staticModelStorage.FindVALRecord(l_filePath).lock())
	{
		// 参照カウントの加算
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!AddStaticModelReferenceCount(l_record), "登録済みStaticModelの参照数加算に失敗したため、StaticModel読み込み処理に失敗しました。", false);

		a_staticModelLoadResult.m_storageID			= l_record->GetVALStorageID();
		a_staticModelLoadResult.m_staticModelRecord = l_record;

		return true;
	}

	// 既にPending中のStaticModelなら再度ロード申請する必要がないのでreturn
	if (const auto& l_itr = m_pendingStaticModelBatchUploadRecordMap.find(l_filePath);
		l_itr != m_pendingStaticModelBatchUploadRecordMap.end())
	{
		const auto& l_staticModelRecord = l_itr->second.m_staticModelRecord;

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_staticModelRecord, "Pending中のStaticModelRecordが無効のため、StaticModel読み込み処理に失敗しました。", false);

		l_staticModelRecord->AddReferenceCount();

		a_staticModelLoadResult.m_storageID			= l_staticModelRecord->GetVALStorageID();
		a_staticModelLoadResult.m_staticModelRecord = l_staticModelRecord;

		return true;
	}

	return false;
}