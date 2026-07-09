#include "StaticStructuredBuffer.h"

FWK::Graphics::StaticStructuredBuffer::StaticStructuredBuffer() = default;
FWK::Graphics::StaticStructuredBuffer::~StaticStructuredBuffer()
{
	Release();
}

FWK::Graphics::StaticStructuredBuffer::StaticStructuredBuffer(StaticStructuredBuffer&& a_other) noexcept : 
	StructuredBufferBase(std::move(a_other))
{}

FWK::Graphics::StaticStructuredBuffer& FWK::Graphics::StaticStructuredBuffer::operator=(StaticStructuredBuffer && a_other) noexcept
{
	if (this == &a_other) { return *this; }

	// 自身のリソースを解放してからMove処理を行う
	Release();

	StructuredBufferBase::operator=(std::move(a_other));
	
	return *this;
}

bool FWK::Graphics::StaticStructuredBuffer::ReserveRelease(const UINT64& a_retiredFenceValue, ResourceReleaseContext& a_resourceReleaseContext)
{
	return ReserveReleaseCurrentResource(a_retiredFenceValue, a_resourceReleaseContext);
}

void FWK::Graphics::StaticStructuredBuffer::Release()
{
	// 既に解放するものがなければreturn;
	if (!GetREFBufferGPUResource().m_resource &&
		GetVALSRVDescriptorIndex() == DescriptorHeap::k_invalidDescriptorIndex)
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
	FWK_ASSERT_RETURN_IF(!ReserveRelease(l_retiredFenceValue, l_resourceReleaseContext), "StaticStructuredBufferの自動遅延解放登録に失敗しました。");
}