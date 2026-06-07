#include "ResourceReleaseContext.h"

bool FWK::Graphics::ResourceReleaseContext::ReserveDeferredReleaseGPUResourceRecord(Struct::GPUResourceReleaseRecord&& a_releaseRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED		   (!IsValidGPUResourceReleaseRecord(a_releaseRecord), "GPUResourceRecordが無効のため、遅延解放登録に失敗しました。", false);
	m_gpuResourceReleaseRecordList.emplace_back(std::move(a_releaseRecord));

	return true;
}

bool FWK::Graphics::ResourceReleaseContext::ReserveDeferredReleaseSRVDescriptorIndex(Struct::DescriptorIndexReleaseRecord&& a_releaseRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED				  (!IsValidDescriptorIndexReleaseRecord(a_releaseRecord), "SRV用DescriptorIndexが無効のため、遅延解放予約に失敗しました。", false);
	m_srvDescriptorIndexReleaseRecordList.emplace_back(std::move(a_releaseRecord));

	return false;
}

void FWK::Graphics::ResourceReleaseContext::ReleaseAvailableDeferredResources(const DirectCommandQueue& a_directCommandQueue, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	const auto& l_completedFenceValue = a_directCommandQueue.FetchVALCompletedFenceValue();

	// GPUResource本体を解放
	// ComPtrを保持しているRecordをpop_backすることで、GPUResourceの参照が外れる。
	ReleaseAvailableGPUResources(l_completedFenceValue);

	// DescriptorのDescriptorIndexを、それぞれ対応するDescriptorPoolへ返す、
	// RTV/SRV/DSVは別のDescriptorHeapなので、必ず別々のPoolへ返す
	ReleaseAvailableDescriptorIndices(l_completedFenceValue, m_srvDescriptorIndexReleaseRecordList, a_srvDescriptorPool);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ResourceReleaseContext::ReleaseRenderTargetResourceImmediately(const TypeAlias::DescriptorIndex a_rtvDescriptorIndex, TypeAlias::ComPtr<ID3D12Resource2>& a_renderTargetResource, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool) const
{
	// 二重開放をしない
	if (a_rtvDescriptorIndex == Constant::k_invalidDescriptorIndex) { return Constant::k_invalidDescriptorIndex; }

	// SwapChain::ResizeBackBuffer()の前は、BackBufferへの参照が残っていると失敗する
	// そのため、ComPtrを明示的にResetして、CPU側の参照を外す
	a_renderTargetResource.Reset();

	// RTV用DescriptorIndexをDescriptorPoolへ返却する
	a_rtvDescriptorPool.Release(a_rtvDescriptorIndex);

	// 戻り値として無効な値を渡して二重開放を防ぐようにする
	return Constant::k_invalidDescriptorIndex;
}

bool FWK::Graphics::ResourceReleaseContext::IsValidGPUResourceReleaseRecord(const Struct::GPUResourceReleaseRecord& a_releaseRecord) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_releaseRecord.m_gpuResource.m_resource,						   "無効なリソースを解放しようとしています。",													 false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_releaseRecord.m_retiredFenceValue == Constant::k_unusedFenceValue, "無効なフェンス値となっており、解放のタイミングが分かりらないものを解放しようとしています。", false);

	return true;
}
bool FWK::Graphics::ResourceReleaseContext::IsValidDescriptorIndexReleaseRecord(const Struct::DescriptorIndexReleaseRecord& a_releaseRecord) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_releaseRecord.m_retiredFenceValue == Constant::k_invalidDescriptorIndex, "無効なディスクリプタインデックスを解放しようとしています。",								   false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_releaseRecord.m_retiredFenceValue == Constant::k_unusedFenceValue,       "無効なフェンス値となっており、解放のタイミングが分かりらないものを解放しようとしています。", false);

	return true;
}

void FWK::Graphics::ResourceReleaseContext::ReleaseAvailableGPUResources(const UINT64& a_completedFenceValue)
{
	std::size_t l_index = 0ULL;

	while (l_index < m_gpuResourceReleaseRecordList.size())
	{
		// GPUのフェンス値より大きいフェンス値ならまだ解放しない
		if (m_gpuResourceReleaseRecordList[l_index].m_retiredFenceValue > a_completedFenceValue)
		{
			++l_index;
			continue;
		}

		// 解放順は不要なので、末尾要素を移動してpop_backする
		// pop_backされた要素内のComPtrは自然にReleaseされる
		std::swap(m_gpuResourceReleaseRecordList[l_index], m_gpuResourceReleaseRecordList.back());
		m_gpuResourceReleaseRecordList.pop_back();
	}
}