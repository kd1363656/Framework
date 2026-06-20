#include "RenderGraphFrameResource.h"

void FWK::Graphics::RenderGraphFrameResource::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Graphics::RenderGraphFrameResource::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::RenderGraphFrameResource::Create(const Device&			   a_device,
												     const GPUMemoryAllocator& a_gpuMemoryAllocator, 
													 const Struct::ClientSize& a_clientSize,
														   ResourceContext&    a_resourceContext) const
{
	auto& l_rtvDescriptorPool = a_resourceContext.GetMutableREFRTVDescriptorPool();
	auto& l_srvDescriptorPool = a_resourceContext.GetMutableREFSRVDescriptorPool();
	auto& l_dsvDescriptorPool = a_resourceContext.GetMutableREFDSVDescriptorPool();

	for (const auto& l_renderTargetPassTexture : m_renderTargetPassTextureList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderGraphFrameResourceの作成に失敗しました。", false);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture->Create(a_device,
																			 a_gpuMemoryAllocator,
																			 a_clientSize,
																			 l_rtvDescriptorPool,
																			 l_srvDescriptorPool),	
																			 "RenderTargetTextureの作成に失敗しました。",
																			 false);
	}

	for (const auto& l_depthStencilPassTexture : m_depthStencilPassTextureList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_depthStencilPassTexture, "DepthStencilPassTextureが無効のため、RenderGraphFrameResourceの作成に失敗しました。", false);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_depthStencilPassTexture->Create(a_device,
																			 a_gpuMemoryAllocator,
																			 a_clientSize,
																			 l_dsvDescriptorPool),
																			 "DepthStencilTextureの作成に失敗しました。",
																			 false);
	}

	return true;
}

bool FWK::Graphics::RenderGraphFrameResource::Resize(const Device&				a_device, 
													 const GPUMemoryAllocator&	a_gpuMemoryAllocator, 
													 const Struct::ClientSize&	a_clientSize, 
													 const UINT64&				a_retiredFenceValue,
														    ResourceContext&    a_resourceContext) const
{
	auto& l_rtvDescriptorPool      = a_resourceContext.GetMutableREFRTVDescriptorPool     ();
	auto& l_srvDescriptorPool      = a_resourceContext.GetMutableREFSRVDescriptorPool     ();
	auto& l_dsvDescriptorPool      = a_resourceContext.GetMutableREFDSVDescriptorPool     ();
	auto& l_resourceReleaseContext = a_resourceContext.GetMutableREFResourceReleaseContext();

	for (const auto& l_renderTargetPassTexture : m_renderTargetPassTextureList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderGraphFrameResourceのリサイズ処理に失敗しました。", false);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture->Resize(a_device,
																			 a_gpuMemoryAllocator,
																			 a_clientSize,
																		     a_retiredFenceValue,
																			 l_rtvDescriptorPool,
																			 l_srvDescriptorPool,
																			 l_resourceReleaseContext),	
																			 "RenderTargetTextureのリサイズに失敗しました。",
																			 false);
	}

	for (const auto& l_depthStencilPassTexture : m_depthStencilPassTextureList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_depthStencilPassTexture, "DepthStencilPassTextureが無効のため、RenderGraphFrameResourceのリサイズ処理に失敗しました。", false);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_depthStencilPassTexture->Resize(a_device,
																			 a_gpuMemoryAllocator,
																			 a_clientSize,
																			 a_retiredFenceValue,
																			 l_dsvDescriptorPool,
																			 l_resourceReleaseContext),
																			 "DepthStencilTextureのリサイズに失敗しました。",
																			 false);
	}

	return true;
}

void FWK::Graphics::RenderGraphFrameResource::AddRenderTargetPassTexture(const std::shared_ptr<RenderTargetPassTexture>& a_renderTargetPassTexture)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderGraphFrameResourceへの登録に失敗しました。");

	const auto l_renderGraphResourceType = a_renderTargetPassTexture->GetVALRenderGraphResourceType();

	FWK_ASSERT_RETURN_IF_FAILED(l_renderGraphResourceType == Enum::RenderGraphResourceType::Invalid, "RenderTargetPassTextureのRenderGraphResourceTypeが無効のため、RenderGraphFrameResourceへの登録に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(m_renderTargetPassTextureMap.contains(l_renderGraphResourceType),    "同じRenderGraphResourceTypeのRenderTargetPassTextureを二重登録しようとしており、RenderGraphFrameResourceへの登録に失敗しました。");

	m_renderTargetPassTextureList.emplace_back(a_renderTargetPassTexture);
	m_renderTargetPassTextureMap.try_emplace	  (l_renderGraphResourceType, a_renderTargetPassTexture);
}

void FWK::Graphics::RenderGraphFrameResource::AddDepthStencilPassTexture(const std::shared_ptr<DepthStencilPassTexture>& a_depthStencilPassTexture)
{
	FWK_ASSERT_RETURN_IF_FAILED(!a_depthStencilPassTexture, "DepthStencilPassTextureが無効のため、RenderGraphFrameResourceへの登録に失敗しました。");

	const auto l_renderGraphResourceType = a_depthStencilPassTexture->GetVALRenderGraphResourceType();

	FWK_ASSERT_RETURN_IF_FAILED(l_renderGraphResourceType == Enum::RenderGraphResourceType::Invalid, "DepthStencilPassTextureのRenderGraphResourceTypeが無効のため、RenderGraphFrameResourceへの登録に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(m_depthStencilPassTextureMap.contains(l_renderGraphResourceType),    "同じRenderGraphResourceTypeのDepthStencilPassTextureを二重登録しようとしており、RenderGraphFrameResourceへの登録に失敗しました。");

	// RenderTargetPassTextureとDepthStencilPathTextureでキーとして扱うRenderGraphResourceTypeを分けるために
	// RenderTargetPassTextureで使用しているRenderGraphResourceTypeのキーを使用しない
	FWK_ASSERT_RETURN_IF_FAILED(m_renderTargetPassTextureMap.contains(l_renderGraphResourceType), "RenderTargetPassTextureとDepthStencilPassTextureで同じrenderGraphResourceTypeを使っており、RenderGraphFrameResourceへの登録に失敗しました。");

	m_depthStencilPassTextureList.emplace_back(a_depthStencilPassTexture);
	m_depthStencilPassTextureMap.try_emplace  (l_renderGraphResourceType, a_depthStencilPassTexture);
}

std::weak_ptr<FWK::Graphics::RenderTargetPassTexture> FWK::Graphics::RenderGraphFrameResource::FindVALRenderTargetPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const
{
	const auto& l_itr = m_renderTargetPassTextureMap.find(a_renderGraphResourceType);

	if (l_itr == m_renderTargetPassTextureMap.end()) { return {}; }

	return l_itr->second;
}
std::weak_ptr<FWK::Graphics::DepthStencilPassTexture> FWK::Graphics::RenderGraphFrameResource::FindVALDepthStencilPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const
{
	const auto& l_itr = m_depthStencilPassTextureMap.find(a_renderGraphResourceType);

	if (l_itr == m_depthStencilPassTextureMap.end()) { return {}; }

	return l_itr->second;
}