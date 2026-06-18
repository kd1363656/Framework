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

	auto l_staticModelRecord = std::make_shared<Graphics::StaticModelRecord>();

	l_staticModelRecord->SetFilePath      (a_filePath.wstring());
	l_staticModelRecord->SetStorageID     (l_allocateStorageID);
	l_staticModelRecord->SetReferenceCount(Constant::k_defaultAssetReferenceCount);

	// .assetが存在していて、FBXより更新が古くなければ.assetを優先して読み込む
	if (!m_staticModelBinaryConverter.LoadStaticModelAsset(a_filePath, *l_staticModelRecord))
	{
		// .assetが読み込めなければテクスチャをロードする、失敗したらassert
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_loader.LoadStaticModelFile(a_filePath, *l_staticModelRecord), "StaticModel読み込みに失敗したため、バッチモデル登録に失敗しました。。", {});

		// 読み込んだテクスチャのデータを保存、次回以降はバイナリーファイルで読み込めるようにする
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_staticModelBinaryConverter.SaveStaticModelAsset(a_filePath, *l_staticModelRecord), "TextureAssetの保存に失敗しました", {});
	}

	return l_staticModelLoadResult;
}

nlohmann::json FWK::Graphics::StaticModelSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
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