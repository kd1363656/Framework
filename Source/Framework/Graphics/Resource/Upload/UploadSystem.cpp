#include "UploadSystem.h"

void FWK::Graphics::UploadSystem::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::UploadSystem::Create(const Device& a_device)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_copyCommandQueue.Create(a_device), "コピーコマンドキュー作成処理に失敗しました。", false);

	// Deserializeでコピーコマンドアロケータの数を設定している、もしコマンドアロケータ数が0なら
	// リストが空の場合は、UploadSystemJsonConverterのDeserialize処理を確認すること
	for (const auto& l_copyCommandAllocator : m_copyCommandAllocatorList)
	{
		FWK_ASSERT_RETURN_VALUE_IF(!l_copyCommandAllocator,                   "コマンドアロケーターが無効のため、作成処理に失敗しました。", false);
		FWK_ASSERT_RETURN_VALUE_IF(!l_copyCommandAllocator->Create(a_device), "コピーコマンドアロケータ作成処理に失敗しました。",           false);
	}

	FWK_ASSERT_RETURN_VALUE_IF(!m_copyCommandList.Create(a_device), "コピーコマンドリスト作成処理に失敗しました。", false);

	return true;
}

void FWK::Graphics::UploadSystem::SubmitPendingTextureCopyBatchIfNeededAndWait(const TextureSystem& a_textureSystem)
{
	const auto& l_pendingTextureBatchUploadRecordMap = a_textureSystem.GetREFPendingTextureBatchUploadRecordMap();

	if (l_pendingTextureBatchUploadRecordMap.empty()) { return; }

	const auto& l_copyCommandAllocator = FetchMutablePTRCopyCommandAllocator().lock();

	FWK_ASSERT_RETURN_IF(!l_copyCommandAllocator, "使用可能なコピーコマンドアロケータが取得できず、バッチテクスチャコピー送信処理に失敗しました。");

	// コマンドリストなどのリセットなどを行う
	BeforSubmitResourceProcess(*l_copyCommandAllocator);

	// UploadBuffer内に配置した各サブリソースの画像データを
	// D3D12_PLACED_SUBRESOURCE_FOOTPRINTの配置情報に従って、DEFAULTヒープ上のテクスチャリソースへコピーする
	for (const auto& [l_filePath, l_pendingTextureBatchUploadRecord] : l_pendingTextureBatchUploadRecordMap)
	{
		const auto& l_textureRecord = l_pendingTextureBatchUploadRecord.m_textureRecord;

		FWK_ASSERT_RETURN_IF(!l_textureRecord, "TextureRecordが無効になっており、テクスチャコピー処理ができませんでした。");

		const auto& l_textureUploadRecord = l_pendingTextureBatchUploadRecord.m_textureUploadRecord;
 
		// DEFAULTバッファーにアプロードバッファーの内容をコピーする
		RecordTextureCopy(l_textureUploadRecord.m_layoutList, l_textureRecord->GetREFGPUResource().m_resource, l_textureUploadRecord.m_uploadBuffer.GetREFUploadBuffer());
	}

	// コマンドリストの実行などを行う
	AfterSubmitResourceProcess(*l_copyCommandAllocator);
}
void FWK::Graphics::UploadSystem::SubmitPendingStaticModelBatchIfNeededAndWait(const StaticModelSystem& a_staticModelSystem)
{
	const auto& l_pendingModelBatchUploadRecordMap = a_staticModelSystem.GetREFPendingModelBatchUploadRecordMap();

	if (l_pendingModelBatchUploadRecordMap.empty()) { return; }

	const auto& l_copyCommandAllocator = FetchMutablePTRCopyCommandAllocator().lock();

	FWK_ASSERT_RETURN_IF(!l_copyCommandAllocator, "使用可能なコピーコマンドアロケータが取得できず、StaticModel用BufferResourceのバッチコピーに失敗しました。");

	// コマンドリストなどのリセットなどを行う
	BeforSubmitResourceProcess(*l_copyCommandAllocator);

	for (const auto& [l_filePath, l_pendingStaticModelBatchUploadRecord] : l_pendingModelBatchUploadRecordMap)
	{
		FWK_ASSERT_RETURN_IF(l_pendingStaticModelBatchUploadRecord.m_bufferUploadCommandList.empty(), "StaticModel用BufferUploadCommandListが空のため、StaticModel用BufferResourceのバッチコピーに失敗しました。");

		// バッファーをコピーしていく
		for (const auto& l_bufferUploadCommand : l_pendingStaticModelBatchUploadRecord.m_bufferUploadCommandList)
		{
			RecordBufferCopy(l_bufferUploadCommand);
		}
	}

	// コマンドリストの実行などを行う
	AfterSubmitResourceProcess(*l_copyCommandAllocator);
}
void FWK::Graphics::UploadSystem::SubmitPendingSkeletalAnimationModelBatchIfNeededAndWait(const SkeletalAnimationModelSystem& a_skeletalAnimationModelSystem)
{
	const auto& l_pendingModelBatchUploadRecordMap = a_skeletalAnimationModelSystem.GetREFPendingModelBatchUploadRecordMap();

	if (l_pendingModelBatchUploadRecordMap.empty()) { return; }

	const auto& l_copyCommandAllocator = FetchMutablePTRCopyCommandAllocator().lock();

	FWK_ASSERT_RETURN_IF(!l_copyCommandAllocator, "使用可能なコピーコマンドアロケータが取得できず、SkeletalAnimationModel用BufferResourceのバッチコピーに失敗しました。");

	// CommandAllocatorとCommandListをリセットする
	BeforSubmitResourceProcess(*l_copyCommandAllocator);

	for (const auto& [l_filePath, l_pendingModelBatchUploadRecord] : l_pendingModelBatchUploadRecordMap)
	{
		const auto& l_bufferUploadCommandList = l_pendingModelBatchUploadRecord.m_bufferUploadCommandList;

		FWK_ASSERT_RETURN_IF(l_bufferUploadCommandList.empty(), "SkeletalAnimationModel用BufferUploadCommandListが空のため、BufferResourceのバッチコピーに失敗しました。");

		for (const auto& l_bufferUploadCommand : l_bufferUploadCommandList)
		{
			RecordBufferCopy(l_bufferUploadCommand);
		}
	}

	// CommandListを実行してコピー完了を待つ
	AfterSubmitResourceProcess(*l_copyCommandAllocator);
}

nlohmann::json FWK::Graphics::UploadSystem::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::UploadSystem::AddCommandAllocator(const std::shared_ptr<TypeAlias::CopyCommandAllocator>& a_copyCommandAllocator)
{
	FWK_ASSERT_RETURN_IF(!a_copyCommandAllocator, "無効なコピーコマンドアロケーターです、コピーコマンドアロケーター追加処理に失敗しました。");

	m_copyCommandAllocatorList.emplace_back(a_copyCommandAllocator);
}

void FWK::Graphics::UploadSystem::BeforSubmitResourceProcess(const TypeAlias::CopyCommandAllocator& a_copyCommandAllocator)
{
	// 命令を格納できるようにするためリセット
	a_copyCommandAllocator.Reset();
	m_copyCommandList.Reset(a_copyCommandAllocator);
}
void FWK::Graphics::UploadSystem::AfterSubmitResourceProcess(TypeAlias::CopyCommandAllocator& a_copyCommandAllocator)
{
	m_copyCommandList.Close				  ();
	m_copyCommandQueue.ExecuteCommandLists(m_copyCommandList);

	m_copyCommandQueue.SignalAndTrackAllocator(a_copyCommandAllocator);

	// Batch + Wait方式なので、ここでGPUコピー完了まで待つ
	// この関数を抜けた後はUploadBufferを保持し続ける必要がない
	m_copyCommandQueue.EnsureAllocatorAvailable(a_copyCommandAllocator);	
}

void FWK::Graphics::UploadSystem::RecordTextureCopy(const std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT>& a_layoutList, const TypeAlias::ComPtr<ID3D12Resource2>& a_textureResource, const TypeAlias::ComPtr<ID3D12Resource2>& a_uploadBuffer) const
{
	FWK_ASSERT_RETURN_IF(!a_textureResource,   "コピー先TextureResourceが無効のため、テクスチャコピー記録に失敗しました。");
	FWK_ASSERT_RETURN_IF(!a_uploadBuffer,      "コピー元UploadBufferが無効のため、テクスチャコピー記録に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_layoutList.empty(), "サブリソース配置情報が空のため、テクスチャコピー記録処理に失敗しました。");

	for (auto l_subresourceIndex = 0U; l_subresourceIndex < static_cast<UINT>(a_layoutList.size()); ++l_subresourceIndex)
	{
		// D3D12_TEXTURE_COPY_LOCATIONについての説明
		// pResource        : コピー先になるTextureResource
		// Type             : コピー先はTextureResourceのサブリソースなのでSUBRESOURCE_INDEXを指定する
		// SubresourceIndex : コピー先TextureResourceのサブリソース番号
		const D3D12_TEXTURE_COPY_LOCATION l_destinationCopyLocation = { a_textureResource.Get(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX, l_subresourceIndex };

		// D3D12_TEXTURE_COPY_LOCATIONについての説明
		// pResource        : コピー元になるUploadBuffer
		// Type             : コピー元はUploadBuffer内の配置済みフットプリントなのでPLACED_FOOTPRINTを指定する
		// PlacedFootprint  : UploadBuffer内にあるコピー元サブリソースの配置情報
		const D3D12_TEXTURE_COPY_LOCATION l_sourceCopyLocation = { a_uploadBuffer.Get(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT, a_layoutList[l_subresourceIndex]};

		m_copyCommandList.CopyTextureRegion(l_destinationCopyLocation,
											l_sourceCopyLocation,
										    nullptr,
											k_defaultTextureCopyDestinationX,
											k_defaultTextureCopyDestinationY,
											k_defaultTextureCopyDestinationZ);
	}
}

void FWK::Graphics::UploadSystem::RecordBufferCopy(const StaticStructuredBuffer::BufferUploadCommand& a_bufferUploadCommand) const
{
	FWK_ASSERT_RETURN_IF(!a_bufferUploadCommand.m_destinationBufferResource, "コピー先BufferResourceが無効のため、バッファコピー処理に失敗しました。");

	const auto& l_uploadBuffer = a_bufferUploadCommand.m_bufferUploadRecord.m_uploadBuffer.GetREFUploadBuffer();

	FWK_ASSERT_RETURN_IF(!l_uploadBuffer,                                                                              "コピー元UploadBufferが無効のため、バッファコピー処理に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_bufferUploadCommand.m_bufferUploadRecord.m_bufferSize == UploadBuffer::k_invalidBufferSize, "コピーするBufferサイズが0のため、バッファコピー処理に失敗しました。");
	
	// コピー先とコピー元のリソースを取得
	auto& l_destinationBufferResource = *a_bufferUploadCommand.m_destinationBufferResource.Get();
	auto& l_sourceBufferResource      = *l_uploadBuffer.Get									  ();

	// UPLOADヒープ上にあるバッファをDEFAULTヒープ上にあるバッファにコピー
	m_copyCommandList.CopyBufferRegion(k_bufferCopyDestinationOffset,
									   k_bufferCopySourceOffset,
									   a_bufferUploadCommand.m_bufferUploadRecord.m_bufferSize,
									   l_destinationBufferResource,
									   l_sourceBufferResource);
}

std::weak_ptr<FWK::TypeAlias::CopyCommandAllocator> FWK::Graphics::UploadSystem::FetchMutablePTRCopyCommandAllocator()
{
	FWK_ASSERT_RETURN_VALUE_IF(m_copyCommandAllocatorList.empty(),                                      "コピーコマンドアロケータリストが空のため、コピーコマンドアロケータ取得処理に失敗しました。",					       {});
	FWK_ASSERT_RETURN_VALUE_IF(m_currentCopyCommandAllocatorIndex >= m_copyCommandAllocatorList.size(), "コピーコマンドアロケータリストの容量を超えたインデックスのため、コピーコマンドアロケータ取得処理に失敗しました。", {});

	const auto& l_copyCommandAllocator = m_copyCommandAllocatorList[m_currentCopyCommandAllocatorIndex];

	FWK_ASSERT_RETURN_VALUE_IF(!l_copyCommandAllocator, "コピーコマンドアロケータが無効のため、コピーコマンドアロケータ取得処理に失敗しました。", {});

	// もしWaitが必要なコマンドアロケータならWaitする
	m_copyCommandQueue.EnsureAllocatorAvailable(*l_copyCommandAllocator);

	// 次のコピーコマンドアロケータを使用するようにインデックスを更新
	m_currentCopyCommandAllocatorIndex = (m_currentCopyCommandAllocatorIndex + k_copyCommandAllocatorIndexIncrement) % m_copyCommandAllocatorList.size();

	return l_copyCommandAllocator;
}