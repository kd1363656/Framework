#include "TextureRecord.h"

bool FWK::Graphics::TextureRecord::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_gpuResource.m_resource,					            "TextureRecordのGPUResourceが無効のため、遅延解放登録に失敗しました。",		   false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_srvDescriptorIndex == Constant::k_invalidStorageID, "TextureRecordのSRVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_retiredFenceValue  == Constant::k_unusedFenceValue, "FenceValueが無効のため、TextureRecordの遅延解放登録に失敗しました。",		   false);

	// GPUResourceはQueueへ所有権を移す
	// Queue内のRecordが消えるタイミングでComPtrが自然にReleaseされる
	Struct::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

	l_gpuResourceReleaseRecord.m_gpuResource	   = std::move(m_gpuResource);
	l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	Struct::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

	l_srvDescriptorIndexReleaseRecord.m_descriptorIndex	  = m_srvDescriptorIndex;	
	l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)),		    "TextureRecordのGPUResourceを遅延解放登録できませんでした。",		 false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseSRVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "TextureRecordのSRVDescriptorIndexを遅延解放登録できませんでした。", false);

	// 二解放を防ぐため、Queueへ渡したDescriptorIndexは無効化する
	m_srvDescriptorIndex = Constant::k_invalidStorageID;

	return true;
}