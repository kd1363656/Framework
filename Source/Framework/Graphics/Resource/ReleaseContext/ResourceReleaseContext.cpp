#include "ResourceReleaseContext.h"

bool FWK::Graphics::ResourceReleaseContext::PushSRVDescriptorIndex(Struct::DescriptorIndexReleaseRecord&& a_releaseRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED				  (!IsValidDescriptorIndexReleaseRecord(a_releaseRecord), "SRV用DescriptorIndexReleaseRecordが無効のため、遅延解放Queueへの登録に失敗しました。", false);
	m_srvDescriptorIndexReleaseRecordList.emplace_back(std::move(a_releaseRecord));

	return false;
}

void FWK::Graphics::ResourceReleaseContext::ReleaseAvailableDeferredResources(const DirectCommandQueue& a_directCommandQueue, TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	const auto& l_completedFenceValue = a_directCommandQueue.FetchVALCompletedFenceValue();

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

bool FWK::Graphics::ResourceReleaseContext::IsValidDescriptorIndexReleaseRecord(const Struct::DescriptorIndexReleaseRecord& a_releaseRecord) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_releaseRecord.m_retiredFenceValue == Constant::k_invalidDescriptorIndex, "無効なディスクリプタインデックスを解放しようとしています。",								   false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_releaseRecord.m_retiredFenceValue == Constant::k_unusedFenceValue,       "無効なフェンス値となっており、解放のタイミングが分かりらないものを解放しようとしています。", false);

	return true;
}