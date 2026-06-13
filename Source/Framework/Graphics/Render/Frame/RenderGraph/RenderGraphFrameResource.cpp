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

bool FWK::Graphics::RenderGraphFrameResource::Create(const Device&			             a_device,
												     const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
													 const Struct::ClientSize&           a_clientSize,
														   TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
														   TypeAlias::SRVDescriptorPool& a_srvDescriptorPool) const
{
	for (const auto& l_renderTargetPassTexture : m_renderTargetPassTextureList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderGraphFrameResourceの作成に失敗しました。", false);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture->Create(a_device,
																			 a_gpuMemoryAllocator,
																			 a_clientSize,
																			 a_rtvDescriptorPool,
																			 a_srvDescriptorPool),	
																			 "RenderTargetTextureの作成に失敗しました。",
																			 false);
	}

	return true;
}

bool FWK::Graphics::RenderGraphFrameResource::Resize(const Device&						 a_device, 
													 const GPUMemoryAllocator&			 a_gpuMemoryAllocator, 
													 const Struct::ClientSize&			 a_clientSize, 
													 const UINT64&						 a_retiredFenceValue,
														   TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, 
														   TypeAlias::SRVDescriptorPool& a_srvDescriptorPool,
														   ResourceReleaseContext&		 a_resourceReleaseContext) const
{
	for (const auto& l_renderTargetPassTexture : m_renderTargetPassTextureList)
	{
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture, "RenderTargetPassTextureが無効のため、RenderGraphFrameResourceのリサイズ処理作成に失敗しました。", false);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_renderTargetPassTexture->Resize(a_device,
																			 a_gpuMemoryAllocator,
																			 a_clientSize,
																		     a_retiredFenceValue,
																			 a_rtvDescriptorPool,
																			 a_srvDescriptorPool,
																			 a_resourceReleaseContext),	
																			 "RenderTargetTextureのリサイズに失敗しました。",
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

std::weak_ptr<FWK::Graphics::RenderTargetPassTexture> FWK::Graphics::RenderGraphFrameResource::FindVALRenderTargetPassTexture(const Enum::RenderGraphResourceType a_renderGraphResourceType) const
{
	const auto& l_itr = m_renderTargetPassTextureMap.find(a_renderGraphResourceType);

	if (l_itr == m_renderTargetPassTextureMap.end()) { return {}; }

	return l_itr->second;
}