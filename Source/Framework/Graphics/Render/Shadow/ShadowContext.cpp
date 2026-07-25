#include "ShadowContext.h"

void FWK::Graphics::ShadowContext::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::ShadowContext::Create(const Device&                             a_device, 
	                                      const GPUMemoryAllocator&                 a_gpuMemoryAllocator, 
	                                            TypeAlias::DSVDescriptorPool&       a_dsvDescriptorPool, 
	                                            TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_cascadeShadowMap.Create(a_device,
			                                              a_gpuMemoryAllocator,
			                                              a_dsvDescriptorPool,
			                                              a_cbvSRVUAVDescriptorPool),
		                                                  "CascadeShadowMapの作成処理に失敗したため、ShadowContextの作成処理に失敗しました。",
		                                                  false);

	return true;
}

nlohmann::json FWK::Graphics::ShadowContext::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}