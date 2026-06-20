#include "RenderTargetPassTexture.h"

void FWK::Graphics::RenderTargetPassTexture::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}
nlohmann::json FWK::Graphics::RenderTargetPassTexture::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::RenderTargetPassTexture::Create(const Device&			            a_device, 
												    const GPUMemoryAllocator&           a_gpuMemoryAllocator,
													const Struct::ClientSize&           a_clientSize,
														  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
														  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_format == DXGI_FORMAT_UNKNOWN, "RenderTargetPassTextureのFormatが無効のため、作成処理に失敗しました。", false);

	const auto l_width  = FetchVALTextureWidth (a_clientSize.m_width);
	const auto l_height = FetchVALTextureHeight(a_clientSize.m_height);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_renderTargetTexture.Create(a_device,
																    a_gpuMemoryAllocator,
																    m_format,
																    l_width,
																    l_height,
																    m_clearColor,
																    a_rtvDescriptorPool,
																    a_srvDescriptorPool),
																    "RenderTargetPassTexture内部のRenderTargetTexture作成に失敗しました。",
																    false);

	return true;
}

bool FWK::Graphics::RenderTargetPassTexture::Resize(const Device&			            a_device, 
												    const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
												    const Struct::ClientSize&           a_clientSize, 
													const UINT64&			            a_retiredFenceValue, 
														  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
														  TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
														  ResourceReleaseContext&       a_resourceReleaseContext)
{
	const auto l_width  = FetchVALTextureWidth (a_clientSize.m_width);
	const auto l_height = FetchVALTextureHeight(a_clientSize.m_height);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_renderTargetTexture.Resize(a_device,
																    a_gpuMemoryAllocator,
																    a_retiredFenceValue,
																    l_width,
																    l_height,
																    a_rtvDescriptorPool,
																    a_srvDescriptorPool,
																    a_resourceReleaseContext),
																    "RenderTargetPassTexture内部のRenderTargetTextureリサイズに失敗しました。",
																    false);

	return true;
}

UINT FWK::Graphics::RenderTargetPassTexture::FetchVALTextureWidth(const UINT a_width) const
{
	// クライアント領域のサイズをテクスチャとして使わないなら
	// 自身が持っているテクスチャの幅を適用
	if (m_isFixedSize) { return m_width; }

	return a_width;
}
UINT FWK::Graphics::RenderTargetPassTexture::FetchVALTextureHeight(const UINT a_height) const
{
	// クライアント領域のサイズをテクスチャとして使わないなら
	// 自身が持っているテクスチャの高さを適用
	if (m_isFixedSize) { return m_height; }

	return a_height;
}