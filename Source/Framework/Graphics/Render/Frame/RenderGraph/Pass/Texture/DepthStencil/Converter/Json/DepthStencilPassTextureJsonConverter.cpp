#include "DepthStencilPassTextureJsonConverter.h"

void FWK::Converter::DepthStencilPassTextureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DepthStencilPassTexture& a_depthStencilPassTexture) const
{
	if (a_rootJson.is_null()) { return; }

	Struct::DepthStencilTextureSettings l_depthStencilTextureSettings = {};
																								 
	l_depthStencilTextureSettings.m_resourceFormat    = a_rootJson.value(k_resourceFormatJsonKey,    Struct::DepthStencilTextureSettings::k_defaultResourceFormat);
	l_depthStencilTextureSettings.m_dsvFormat         = a_rootJson.value(k_dsvFormatJsonKey,         Struct::DepthStencilTextureSettings::k_defaultDSVFormat);
	l_depthStencilTextureSettings.m_srvFormat         = a_rootJson.value(k_srvFormatJsonKey,         Struct::DepthStencilTextureSettings::k_defaultSRVFormat);
	l_depthStencilTextureSettings.m_depthClearValue   = a_rootJson.value(k_depthClearValueJsonKey,   Constant::k_defaultDepthClearValue);
	l_depthStencilTextureSettings.m_arraySize         = a_rootJson.value(k_arraySizeJsonKey,         Struct::DepthStencilTextureSettings::k_defaultArraySize);
	l_depthStencilTextureSettings.m_mipLevels         = a_rootJson.value(k_mipLevelsJsonKey,         Struct::DepthStencilTextureSettings::k_defaultMipLevels);
	l_depthStencilTextureSettings.m_sampleCount       = a_rootJson.value(k_sampleCountJsonKey,       Constant::k_defaultSampleCount);
	l_depthStencilTextureSettings.m_sampleQuality     = a_rootJson.value(k_sampleQualityJsonKey,     Constant::k_defaultSampleQuality);
	l_depthStencilTextureSettings.m_stencilClearValue = a_rootJson.value(k_stencilClearValueJsonKey, Constant::k_defaultStencilClearValue);

	const auto l_renderGraphDepthStencilType = a_rootJson.value(k_renderGraphDepthStencilTypeJsonKey, Enum::RenderGraphDepthStencilType::Invalid);

	const auto l_width  = a_rootJson.value(k_widthJsonKey,  TextureBinaryConverter::k_emptyTextureWidth);
	const auto l_height = a_rootJson.value(k_heightJsonKey, TextureBinaryConverter::k_emptyTextureHeight);

	const auto l_isFixedSize = a_rootJson.value(k_isFixedSizeJsonKey, false);
	
	a_depthStencilPassTexture.SetDepthStencilTextureSettings(l_depthStencilTextureSettings);
	a_depthStencilPassTexture.SetRenderGraphDepthStencilType(l_renderGraphDepthStencilType);

	a_depthStencilPassTexture.SetWidth (l_width);
	a_depthStencilPassTexture.SetHeight(l_height);

	a_depthStencilPassTexture.SetIsFixedSize(l_isFixedSize);
}

nlohmann::json FWK::Converter::DepthStencilPassTextureJsonConverter::Serialize(const Graphics::DepthStencilPassTexture & a_depthStencilPassTexture) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_depthStencilTextureSettings = a_depthStencilPassTexture.GetREFDepthStencilTextureSettings();

	l_rootJson[k_resourceFormatJsonKey]    = l_depthStencilTextureSettings.m_resourceFormat;
	l_rootJson[k_dsvFormatJsonKey]         = l_depthStencilTextureSettings.m_dsvFormat;
	l_rootJson[k_srvFormatJsonKey]         = l_depthStencilTextureSettings.m_srvFormat;
	l_rootJson[k_depthClearValueJsonKey]   = l_depthStencilTextureSettings.m_depthClearValue;
	l_rootJson[k_arraySizeJsonKey]         = l_depthStencilTextureSettings.m_arraySize;
	l_rootJson[k_mipLevelsJsonKey]         = l_depthStencilTextureSettings.m_mipLevels;
	l_rootJson[k_sampleCountJsonKey]       = l_depthStencilTextureSettings.m_sampleCount;
	l_rootJson[k_sampleQualityJsonKey]     = l_depthStencilTextureSettings.m_sampleQuality;
	l_rootJson[k_stencilClearValueJsonKey] = l_depthStencilTextureSettings.m_stencilClearValue;

	l_rootJson[k_renderGraphDepthStencilTypeJsonKey] = a_depthStencilPassTexture.GetVALRenderGraphDepthStencilType();
	l_rootJson[k_widthJsonKey]					     = a_depthStencilPassTexture.GetVALWidth                      ();
	l_rootJson[k_heightJsonKey]					     = a_depthStencilPassTexture.GetVALHeight			          ();
	l_rootJson[k_isFixedSizeJsonKey]			     = a_depthStencilPassTexture.GetVALIsFixedSize			      ();
	
	return l_rootJson;
}