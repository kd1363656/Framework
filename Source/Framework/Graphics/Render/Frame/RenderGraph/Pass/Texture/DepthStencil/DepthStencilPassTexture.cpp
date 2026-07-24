#include "DepthStencilPassTexture.h"

void FWK::Graphics::DepthStencilPassTexture::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::DepthStencilPassTexture::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::DepthStencilPassTexture::Create(const Device&                             a_device, 
	                                                const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
	                                                const Window::ClientSize&                 a_clientSize, 
	                                                      TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool,
	                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_resourceFormat == DXGI_FORMAT_UNKNOWN, "DepthStencilPassTextureのFormatが無効のため、作成処理に失敗しました。", false);

	const auto l_width  = FetchVALPassTextureWidth (a_clientSize.m_width);
	const auto l_height = FetchVALPassTextureHeight(a_clientSize.m_height);

	FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(l_width, l_height), "DepthStencilTextureの作成サイズが無効のため、作成処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF(!m_depthStencilTexture.Create(a_device,
															 a_gpuMemoryAllocator,
															 m_depthStencilTextureSettings,
															 l_width,
															 l_height,
															 a_dsvDescriptorPool,
		                                                     a_cbvSRVUAVDescriptorPool),
															 "DepthStencilTexture内部のDepthStencilTexture作成に失敗しました。",
															 false);

	return true;
}

bool FWK::Graphics::DepthStencilPassTexture::Resize(const Device&			                  a_device,
													const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
													const Window::ClientSize&                 a_clientSize,
													const UINT64&			                  a_retiredFenceValue, 
														  TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool, 
	                                                      TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
														  ResourceReleaseContext&	          a_resourceReleaseContext)
{
	const auto l_width  = FetchVALPassTextureWidth (a_clientSize.m_width);
	const auto l_height = FetchVALPassTextureHeight(a_clientSize.m_height);

	FWK_ASSERT_RETURN_VALUE_IF(!Utility::IsValidTextureSize(l_width, l_height), "DepthStencilTextureの作成サイズが無効のため、リサイズ処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF(!m_depthStencilTexture.Resize(a_device,
															 a_gpuMemoryAllocator,
															 a_retiredFenceValue,
															 l_width,	
															 l_height,
															 a_dsvDescriptorPool,
		                                                     a_cbvSRVUAVDescriptorPool,
															 a_resourceReleaseContext),
															 "DepthStencilPassTexture内部のDepthStencilTextureリサイズに失敗しました。",
															 false);

	return true;
}