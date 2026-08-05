#include "SkeletalAnimationModelSystem.h"

void FWK::Graphics::SkeletalAnimationModelSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::SkeletalAnimationModelSystem::Create()
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_modelStorage.Create(), "AssetStorageの作成に失敗したため、SkeletalAnimationModelSystemの作成処理に失敗しました。", false);

	return true;
}

FWK::Struct::SkeletalAnimationModelLoadResult FWK::Graphics::SkeletalAnimationModelSystem::LoadSkeletalAnimationModelForBatchUpload(const Device&                             a_device, 
	                                                                                                                                const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
	                                                                                                                                const std::filesystem::path&              a_filePath, 
	                                                                                                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	Struct::SkeletalAnimationModelLoadResult l_skeletalAnimationModelLoadResult = {};

	// .fbxでない、もしくは存在しないなら読み込まない
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "SkeletalAnimationModelのFBXファイルが存在しません。", l_skeletalAnimationModelLoadResult);

	// Storage登録済み、またはGPUアップロード町の場合は既存Recordを返す
	if (TryResolveCachedSkeletalAnimationModelResult(a_filePath, l_skeletalAnimationModelLoadResult)) { return l_skeletalAnimationModelLoadResult; }

	const auto l_allocateStorageID = m_modelStorage.AllocateStorageID();

	FWK_ASSERT_RETURN_VALUE_IF(l_allocateStorageID == Constant::k_invalidStorageID, "StorageIDの割り当てに失敗したため、SkeletalAnimationModel読み込み処理に失敗しました。", l_skeletalAnimationModelLoadResult);

	auto l_skeletalAnimationModelRecord = std::make_shared<SkeletalAnimationModelRecord>();

	l_skeletalAnimationModelRecord->SetFilePath      (a_filePath.wstring());
	l_skeletalAnimationModelRecord->SetStorageID     (l_allocateStorageID);
	l_skeletalAnimationModelRecord->SetReferenceCount(AssetRecordBase::k_initialAssetReferenceCount);

	// 有効な.assetを読み込めなかった場合はFBXから作成する
	if (!m_binaryConverter.LoadAsset(a_filePath, *l_skeletalAnimationModelRecord))
	{
		if (!m_loader.LoadSkeletalAnimationModelFile(a_filePath, *l_skeletalAnimationModelRecord))
		{
			m_modelStorage.ReleaseStorageID(l_allocateStorageID);

			FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelのFBX読み込みに失敗しました。", l_skeletalAnimationModelLoadResult);
		}

		if (!BuildSkeletalAnimationModelAssetData(a_filePath, *l_skeletalAnimationModelRecord))
		{
			m_modelStorage.ReleaseStorageID(l_allocateStorageID);

			FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelのAssetData構築に失敗しました。", l_skeletalAnimationModelLoadResult);
		}
	}

	// TextureとGPUBuffer用UploadCommandを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!BuildSkeletalAnimationModelRuntimeData(l_skeletalAnimationModelRecord,
		                                                               a_device,
		                                                               a_gpuMemoryAllocator,
		                                                               a_filePath,
		                                                               l_allocateStorageID,
		                                                               a_cbvSRVUAVDescriptorPool),
		                                                               "SkeletalAnimationModelのRuntimeData構築に失敗しました。",
		                                                               l_skeletalAnimationModelLoadResult);

	l_skeletalAnimationModelLoadResult.m_storageID                    = l_skeletalAnimationModelRecord->GetVALStorageID();
	l_skeletalAnimationModelLoadResult.m_skeletalAnimationModelRecord = l_skeletalAnimationModelRecord;

	return l_skeletalAnimationModelLoadResult;
}

nlohmann::json FWK::Graphics::SkeletalAnimationModelSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::SkeletalAnimationModelSystem::RegisterPendingSkeletalAnimationModels()
{
	for (const auto& [l_filePath, l_pendingModelBatchUploadRecord] : m_pendingModelBatchUploadRecordMap)
	{
		const auto& l_skeletalAnimationModelRecord = l_pendingModelBatchUploadRecord.m_skeletalAnimationModelRecord;

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord,                                            "SkeletalAnimationModelRecordが無効のため、Pending中のモデル登録に失敗しました。");
		FWK_ASSERT_RETURN_IF(!m_modelStorage.RegisterRecord(l_skeletalAnimationModelRecord, l_filePath), "SkeletalAnimationModelRecordのStorage登録に失敗しました。");
	}

	// GPUコピー完了後はUploadBufferを保持する必要がないため破棄する
	m_pendingModelBatchUploadRecordMap.clear();
}

bool FWK::Graphics::SkeletalAnimationModelSystem::AddSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>&a_skeletalAnimationModelRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_modelStorage.AddReferenceCount(a_skeletalAnimationModelRecord), "AssetStorageでの参照数加算に失敗したため、SkeletalAnimationModel参照数加算に失敗しました。", false);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::SubtractSkeletalAnimationModelReferenceCount(const std::weak_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_modelStorage.SubtractReferenceCount(a_skeletalAnimationModelRecord, a_directCommandQueue, a_resourceReleaseContext), "AssetStorageでの参照数減算に失敗したため、SkeletalAnimationModel参照数減算に失敗しました。", false);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::BuildSkeletalAnimationModelAssetData(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	// 頂点とインデックスをGPUで扱いやすい配置へ最適化する
	FWK_ASSERT_RETURN_VALUE_IF(!m_meshOptimizer.OptimizeModelRecord(a_skeletalAnimationModelRecord), "SkeletalAnimationModelMeshの最適化に失敗しました。", false);

	// 最適化済みの頂点とインデックスからMeshletDataを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!m_meshletBuilder.BuildModelRecordMeshletData(a_skeletalAnimationModelRecord), "SkeletalAnimationModelMeshletDataの作成に失敗しました。", false);

	// 次回以降FBX解析を省略できるように.assetへ保存する
	FWK_ASSERT_RETURN_VALUE_IF(!m_binaryConverter.SaveAsset(a_filePath, a_skeletalAnimationModelRecord), "SkeletalAnimationModelAssetの保存に失敗しました。", false);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::BuildSkeletalAnimationModelRuntimeData(const std::shared_ptr<SkeletalAnimationModelRecord>& a_skeletalAnimationModelRecord,
	                                                                                     const Device&                                        a_device, 
	                                                                                     const GPUMemoryAllocator&                            a_gpuMemoryAllocator, 
	                                                                                     const std::filesystem::path&                         a_filePath, 
	                                                                                     const TypeAlias::StorageID                           a_storageID,
	                                                                                           TypeAlias::CBVSRVUAVDescriptorPool&            a_cbvSRVUAVDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、RuntimeData構築に失敗しました。", false);

	Struct::SkeletalAnimationModelBatchUploadRecord l_skeletalAnimationModelBatchUploadRecord = {};

	// Materialが参照するTextureを読み込む
	m_materialRuntimeTextureBuilder.BuildMaterialRuntimeTextures(a_filePath, *a_skeletalAnimationModelRecord);

	// 共通MeshBufferとBonePaletteBufferのUploadCommandを作成する
	if (!CreateSkeletalAnimationModelBatchUploadRecord(a_skeletalAnimationModelRecord,
													   a_device,
													   a_gpuMemoryAllocator,
													   a_cbvSRVUAVDescriptorPool,
													   l_skeletalAnimationModelBatchUploadRecord))
	{
		m_modelStorage.ReleaseStorageID(a_storageID);

		FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelのBatchUploadRecord作成に失敗しました。", false);
	}

	// unordered_mapに既に挿入されていればreturn
	if (!m_pendingModelBatchUploadRecordMap.try_emplace(a_filePath.wstring(), std::move(l_skeletalAnimationModelBatchUploadRecord)).second)
	{
		m_modelStorage.ReleaseStorageID(a_storageID);

		FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelのPendingMap登録に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::CreateSkeletalAnimationModelBatchUploadRecord(const std::shared_ptr<SkeletalAnimationModelRecord>&   a_skeletalAnimationModelRecord, 
	                                                                                            const Device&                                          a_device,
	                                                                                            const GPUMemoryAllocator&                              a_gpuMemoryAllocator, 
	                                                                                                  TypeAlias::CBVSRVUAVDescriptorPool&              a_cbvSRVUAVDescriptorPool, 
	                                                                                                  Struct::SkeletalAnimationModelBatchUploadRecord& a_skeletalAnimationModelBatchUploadRecord) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、BatchUploadRecordの作成に失敗しました。", false);

	a_skeletalAnimationModelBatchUploadRecord.m_skeletalAnimationModelRecord = a_skeletalAnimationModelRecord;

	FWK_ASSERT_RETURN_VALUE_IF(!m_batchUploadRecordBuilder.CreateSkeletalAnimationModelBatchUploadRecord(a_device,
			                                                                                             a_gpuMemoryAllocator,
			                                                                                             a_skeletalAnimationModelBatchUploadRecord.m_bufferUploadCommandList,
			                                                                                             a_cbvSRVUAVDescriptorPool,
			                                                                                             *a_skeletalAnimationModelRecord), 
		                                                                                                 "SkeletalAnimationModel用BufferUploadCommandの作成に失敗しました。",
		                                                                                                 false);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::TryResolveCachedSkeletalAnimationModelResult(const std::filesystem::path& a_filePath, Struct::SkeletalAnimationModelLoadResult& a_skeletalAnimationModelLoadResult)
{
	const auto& l_filePath = a_filePath.wstring();

	// GPUアップロード待ち、Storageへ正式登録済みの場合参照カウントを増やしてStorageIDとポインタを渡す
	if (const auto& l_record = m_modelStorage.FindVALRecord(l_filePath).lock())
	{
		FWK_ASSERT_RETURN_VALUE_IF(!AddSkeletalAnimationModelReferenceCount(l_record), "登録済みSkeletalAnimationModelの参照数加算に失敗しました。", false);

		a_skeletalAnimationModelLoadResult.m_storageID                    = l_record->GetVALStorageID();
		a_skeletalAnimationModelLoadResult.m_skeletalAnimationModelRecord = l_record;

		return true;
	}

	if (const auto& l_itr = m_pendingModelBatchUploadRecordMap.find(l_filePath); l_itr != m_pendingModelBatchUploadRecordMap.end())
	{
		const auto& l_skeletalAnimationModelRecord = l_itr->second.m_skeletalAnimationModelRecord;

		FWK_ASSERT_RETURN_VALUE_IF(!l_skeletalAnimationModelRecord, "Pending中のSkeletalAnimationModelRecordが無効です。", false);

		l_skeletalAnimationModelRecord->AddReferenceCount();

		a_skeletalAnimationModelLoadResult.m_storageID                    = l_skeletalAnimationModelRecord->GetVALStorageID();
		a_skeletalAnimationModelLoadResult.m_skeletalAnimationModelRecord = l_skeletalAnimationModelRecord;

		return true;
	}

	return false;
}
