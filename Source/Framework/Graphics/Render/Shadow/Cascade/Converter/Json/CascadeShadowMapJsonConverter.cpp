#include "CascadeShadowMapJsonConverter.h"

void FWK::Converter::CascadeShadowMapJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::CascadeShadowMap& a_cascadeShadowMap) const
{
	if (a_rootJson.is_null()) { return; }

	// CascadeShadowMapが保持しているTexture設定へ、
	// このConverterから直接デシリアライズする。
	Struct::DepthStencilTextureSettings l_depthStencilTextureSettings = {};
	
	l_depthStencilTextureSettings.m_resourceFormat = a_rootJson.value(k_resourceFormatJsonKey, Constant::k_cascadeShadowMapDefaultResourceFormat);
	l_depthStencilTextureSettings.m_dsvFormat      = a_rootJson.value(k_dsvFormatJsonKey,      Constant::k_cascadeShadowMapDefaultDSVFormat);
	l_depthStencilTextureSettings.m_srvFormat      = a_rootJson.value(k_srvFormatJsonKey,      Constant::k_cascadeShadowMapDefaultSRVFormat);

	l_depthStencilTextureSettings.m_depthClearValue   = a_rootJson.value(k_depthClearValueJsonKey,   Constant::k_defaultDepthClearValue);
	l_depthStencilTextureSettings.m_stencilClearValue = a_rootJson.value(k_stencilClearValueJsonKey, Constant::k_defaultStencilClearValue);

	l_depthStencilTextureSettings.m_arraySize = a_rootJson.value(k_maxCascadeCountJsonKey, Constant::k_cascadeShadowMapDefaultMAXCascadeCount);
	l_depthStencilTextureSettings.m_mipLevels = a_rootJson.value(k_mipLevelsJsonKey,       Struct::DepthStencilTextureSettings::k_defaultMipLevels);

	l_depthStencilTextureSettings.m_sampleCount   = a_rootJson.value(k_sampleCountJsonKey,   Constant::k_defaultSampleCount);
	l_depthStencilTextureSettings.m_sampleQuality = a_rootJson.value(k_sampleQualityJsonKey, Constant::k_defaultSampleQuality);

	const auto l_sampleDepthBias = a_rootJson.value(k_sampleDepthBiasJsonKey, Constant::k_cascadeShadowMapDefaultSampleDepthBias);
	const auto l_resolution      = a_rootJson.value(k_resolutionJsonKey,      Constant::k_cascadeShadowMapDefaultResolution);

	a_cascadeShadowMap.SetDepthStencilTextureSettings(l_depthStencilTextureSettings);
	a_cascadeShadowMap.SetResolution                 (l_resolution);
	a_cascadeShadowMap.SetSampleDepthBias            (l_sampleDepthBias);
}

nlohmann::json FWK::Converter::CascadeShadowMapJsonConverter::Serialize(const Graphics::CascadeShadowMap & a_cascadeShadowMap) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_depthStencilTextureSettings = a_cascadeShadowMap.GetREFDepthStencilTextureSettings();

	l_rootJson[k_resourceFormatJsonKey] = l_depthStencilTextureSettings.m_resourceFormat;
	l_rootJson[k_dsvFormatJsonKey]      = l_depthStencilTextureSettings.m_dsvFormat;
	l_rootJson[k_srvFormatJsonKey]      = l_depthStencilTextureSettings.m_srvFormat;

	l_rootJson[k_depthClearValueJsonKey]   = l_depthStencilTextureSettings.m_depthClearValue;
	l_rootJson[k_stencilClearValueJsonKey] = l_depthStencilTextureSettings.m_stencilClearValue;

	l_rootJson[k_maxCascadeCountJsonKey] = l_depthStencilTextureSettings.m_arraySize;
	l_rootJson[k_mipLevelsJsonKey]       = l_depthStencilTextureSettings.m_mipLevels;

	l_rootJson[k_sampleCountJsonKey]   = l_depthStencilTextureSettings.m_sampleCount;
	l_rootJson[k_sampleQualityJsonKey] = l_depthStencilTextureSettings.m_sampleQuality;

	l_rootJson[k_sampleDepthBiasJsonKey] = a_cascadeShadowMap.GetVALSampleDepthBias();
	l_rootJson[k_resolutionJsonKey]      = a_cascadeShadowMap.GetVALResolution     ();

	return l_rootJson;
}