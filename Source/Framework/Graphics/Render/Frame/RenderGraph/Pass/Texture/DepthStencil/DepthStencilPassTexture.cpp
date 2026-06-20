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

bool FWK::Graphics::DepthStencilPassTexture::Create(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, const Struct::ClientSize& a_clientSize, TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(m_format == DXGI_FORMAT_UNKNOWN, "DepthStencilPassTextureのFormatが無効のため、作成処理に失敗しました。", false);

	const auto l_width  = Utility::FetchVALPassTextureSizeElement(a_clientSize.m_width,  m_width,  m_isFixedSize);
	const auto l_height = Utility::FetchVALPassTextureSizeElement(a_clientSize.m_height, m_height, m_isFixedSize);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::IsValidTextureSize(l_width, l_height), "DepthStencilTextureの作成サイズが無効のため、作成処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_depthStencilTexture.Create(a_device,
																	a_gpuMemoryAllocator,
																	m_format,
																	l_width,
																	l_height,
																	m_depthClearValue,
																	m_stencilClearValue,
																	a_dsvDescriptorPool),
																    "DepthStencilTexture内部のDepthStencilTexture作成に失敗しました。",
																	false);

	return true;
}

bool FWK::Graphics::DepthStencilPassTexture::Resize(const Device&			            a_device,
													const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
													const Struct::ClientSize&           a_clientSize, 
													const UINT64&			            a_retiredFenceValue, 
														  TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool, 
														  ResourceReleaseContext&	    a_resourceReleaseContext)
{
	const auto l_width  = Utility::FetchVALPassTextureSizeElement(a_clientSize.m_width,  m_width,  m_isFixedSize);
	const auto l_height = Utility::FetchVALPassTextureSizeElement(a_clientSize.m_height, m_height, m_isFixedSize);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::IsValidTextureSize(l_width, l_height), "DepthStencilTextureの作成サイズが無効のため、リサイズ処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_depthStencilTexture.Resize(a_device,
																	a_gpuMemoryAllocator,
																	a_retiredFenceValue,
																	l_width,	
																	l_height,
																	a_dsvDescriptorPool,
																	a_resourceReleaseContext),
																	"DepthStencilPassTexture内部のDepthStencilTextureリサイズに失敗しました。",
																	false);

	return true;
}