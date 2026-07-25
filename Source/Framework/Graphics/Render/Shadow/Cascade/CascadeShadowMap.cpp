#include "CascadeShadowMap.h"

void FWK::Graphics::CascadeShadowMap::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::CascadeShadowMap::Create(const Device & a_device, const GPUMemoryAllocator & a_gpuMemoryAllocator, TypeAlias::DSVDescriptorPool & a_dsvDescriptorPool, TypeAlias::CBVSRVUAVDescriptorPool & a_cbvSRVUAVDescriptorPool)
{
    // Cascade Shadow Mapは通常描画時にShaderから読み取るため、
	// SRVFormatが必須になる。
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_srvFormat == DXGI_FORMAT_UNKNOWN,     "CascadeShadowMapのSRVFormatが無効のため、作成処理に失敗しました。",        false);
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_mipLevels != k_requiredMIPLevelCount, "CascadeShadowMapのMIPLevelsがOneではないため、作成処理に失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(m_depthStencilTextureSettings.m_sampleCount != k_requiredSampleCount, "CascadeShadowMapのSampleCountがOneではないため、作成処理に失敗しました。", false);

	// ShadowMapの解像度から、
	// Shadow描画専用のViewportとScissorRECTを設定する。
	FWK_ASSERT_RETURN_VALUE_IF(!m_renderArea.Setup(m_resolution, m_resolution), "CascadeShadowMap用RenderAreaの設定処理に失敗しました。", false);

	FWK_ASSERT_RETURN_VALUE_IF(!m_depthStencilTexture.Create(a_device,
			                                                 a_gpuMemoryAllocator,
			                                                 m_depthStencilTextureSettings,
			                                                 m_resolution,
			                                                 m_resolution,
			                                                 a_dsvDescriptorPool,
			                                                 a_cbvSRVUAVDescriptorPool),
		                                                     "CascadeShadowMap用DepthStencilTextureの作成処理に失敗しました。",
		                                                     false);

	return true;
}

nlohmann::json FWK::Graphics::CascadeShadowMap::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::CascadeShadowMap::FetchVALCascadeDSVDescriptorIndex(const UINT a_cascadeIndex) const
{
	return m_depthStencilTexture.FetchVALDSVDescriptorIndex(a_cascadeIndex, k_shadowMapMIPSlice);
}
