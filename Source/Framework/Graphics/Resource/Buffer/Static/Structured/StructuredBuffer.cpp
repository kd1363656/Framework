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

	// 自身のリソースを解放してからMove処理を行う
	Release ();
	MoveFrom(std::move(a_other));

	return *this;
}

bool FWK::Graphics::StructuredBuffer::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	// 既に解放するものがなければreturn;
	if (!m_bufferGPUResource.m_resource && 
		m_srvDescriptorIndex == Constant::k_invalidDescriptorIndex) 
	{
		return true;
	}

	FWK_ASSERT_RETURN_VALUE_IF(a_retiredFenceValue == Constant::k_unusedFenceValue, "FenceValueが無効のため、StructuredBufferの遅延解放登録に失敗しました。", false);

	// GPUリソース、ディスクリプタインデックスの適切なタイミングでの解放を予約
	Struct::GPUResourceReleaseRecord l_gpuResourceReleaseRecord = {};

	l_gpuResourceReleaseRecord.m_gpuResource       = std::move(m_bufferGPUResource);
	l_gpuResourceReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	Struct::DescriptorIndexReleaseRecord l_srvDescriptorIndexReleaseRecord = {};

	l_srvDescriptorIndexReleaseRecord.m_descriptorIndex   = m_srvDescriptorIndex;
	l_srvDescriptorIndexReleaseRecord.m_retiredFenceValue = a_retiredFenceValue;

	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseGPUResourceRecord(std::move(l_gpuResourceReleaseRecord)),               "StructuredBufferのGPUResourceを遅延解放Queueへ登録できませんでした。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_resourceReleaseContext.ReserveDeferredReleaseCBVSRVUAVDescriptorIndex(std::move(l_srvDescriptorIndexReleaseRecord)), "StructuredBufferのSRVDescriptorIndexを遅延解放Queueへ登録できませんでした。", false);

	// もう一度開放処理が走らないように初期化
	m_bufferGPUResource  = {};
	m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;

	return true;
}

void FWK::Graphics::StructuredBuffer::ReleaseImmediatelySRVDescriptorIndex(TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	if (m_srvDescriptorIndex == Constant::k_invalidDescriptorIndex) { return; }

	a_cbvSRVUAVDescriptorPool.Release(m_srvDescriptorIndex);

	m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
}

void FWK::Graphics::StructuredBuffer::Release()
{
	// 既に解放するものがなければreturn;
	if (!m_bufferGPUResource.m_resource &&
		m_srvDescriptorIndex == Constant::k_invalidDescriptorIndex)
	{
		return;
	}

	auto& l_graphicsManager = FWK::Graphics::GraphicsManager::GetInstance();

	const auto& l_renderer           = l_graphicsManager.GetREFRenderer   ();
	const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

	auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
	auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

	const auto& l_retiredFenceValue = l_directCommandQueue.FetchREFLastSignaledFenceValue();

	// リソースの解放予約を行う
	FWK_ASSERT_RETURN_IF(!ReserveRelease(l_retiredFenceValue, l_resourceReleaseContext), "StructuredBufferの自動遅延解放登録に失敗しました。");
}

void FWK::Graphics::StructuredBuffer::MoveFrom(StructuredBuffer&& a_other) noexcept
{
	m_bufferGPUResource  = std::move(a_other.m_bufferGPUResource);
	m_srvDescriptorIndex = a_other.m_srvDescriptorIndex;

	a_other.m_bufferGPUResource  = {};
	a_other.m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
}