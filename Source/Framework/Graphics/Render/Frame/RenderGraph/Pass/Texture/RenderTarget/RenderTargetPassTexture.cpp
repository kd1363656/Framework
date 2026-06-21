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

	const auto l_width  = FetchVALPassTextureWidth (a_clientSize.m_width);
	const auto l_height = FetchVALPassTextureHeight(a_clientSize.m_height);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::IsValidTextureSize(l_width, l_height), "RenderTargetPassTextureの作成サイズが無効のため、作成処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_renderTargetTexture.Create(a_device,
																    a_gpuMemoryAllocator,
																    m_format,
																    l_width,
																    l_height,
																    m_clearColor,
																    a_rtvDescriptorPool,
																    a_srvDescriptorPool),
																    "RenderTargetPassTexture内部のRenderTargetTexture作成処理に失敗しました。",
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
	const auto l_width  = FetchVALPassTextureWidth (a_clientSize.m_width);
	const auto l_height = FetchVALPassTextureHeight(a_clientSize.m_height);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::IsValidTextureSize(l_width, l_height), "RenderTargetPassTextureのリサイズ後サイズが無効のため、リサイズ処理に失敗しました。", false);


	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_renderTargetTexture.Resize(a_device,
																    a_gpuMemoryAllocator,
																    a_retiredFenceValue,
																    l_width,
																    l_height,
																    a_rtvDescriptorPool,
																    a_srvDescriptorPool,
																    a_resourceReleaseContext),
																    "RenderTargetPassTexture内部のRenderTargetTextureリサイズ処理に失敗しました。",
																    false);

	return true;
}