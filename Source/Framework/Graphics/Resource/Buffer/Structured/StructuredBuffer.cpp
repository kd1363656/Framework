#include "StructuredBuffer.h"

FWK::Graphics::StructuredBuffer::StructuredBuffer() = default;
FWK::Graphics::StructuredBuffer::~StructuredBuffer()
{
	Release();
}

FWK::Graphics::StructuredBuffer::StructuredBuffer(StructuredBuffer&& a_other) noexcept : 
	m_bufferGPUResource(),
	m_srvDescriptorIndex(Constant::k_invalidDescriptorIndex)
{
	MoveFrom(std::move(a_other));
}

FWK::Graphics::StructuredBuffer& FWK::Graphics::StructuredBuffer::operator=(StructuredBuffer && a_other) noexcept
{
	if (this == &a_other) { return *this; }

	Release();

	MoveFrom(std::move(a_other));

	return *this;
}

bool FWK::Graphics::StructuredBuffer::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_bufferGPUResource.m_resource,							  "StructuredBufferのGPUResourceが無効のため、遅延解放登録に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_srvDescriptorIndex == Constant::k_invalidDescriptorIndex, "StructuredBufferのSRVDescriptorIndexが無効のため、遅延解放登録に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_retiredFenceValue == Constant::k_unusedFenceValue,        "FenceValueが無効のため、StructuredBufferの遅延解放登録に失敗しました。",         false);

	// GPUリソース、ディスクリプタインデックスの適切なタイミングでの解放を予約
	Struct::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

	l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_bufferGPUResource);
	l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	Struct::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

	l_srvDescriptorIndexReleaseRecord.m_descriptorIndex = m_srvDescriptorIndex;
	l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)),         "StructuredBufferのGPUResourceを遅延解放Queueへ登録できませんでした。",        false);
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!a_resourceReleaseContext.ReserveDeferredReleaseSRVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "StructuredBufferのSRVDescriptorIndexを遅延解放Queueへ登録できませんでした。", false);

	return true;
}

void FWK::Graphics::StructuredBuffer::Release()
{
	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	const auto& l_renderer           = l_graphicsManager.GetREFRenderer   ();
	const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

	auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
	auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

	const auto& l_retiredFenceValue = l_directCommandQueue.FetchREFLastSignaledFenceValue();

	FWK_ASSERT_RETURN_IF_FAILED(!ReserveRelease(l_retiredFenceValue, l_resourceReleaseContext), "StructuredBufferの自動遅延解放登録に失敗しました。");
}

void FWK::Graphics::StructuredBuffer::MoveFrom(StructuredBuffer&& a_other) noexcept
{
	m_bufferGPUResource  = std::move(a_other.m_bufferGPUResource);
	m_srvDescriptorIndex = a_other.m_srvDescriptorIndex;

	a_other.m_bufferGPUResource  = {};
	a_other.m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
}