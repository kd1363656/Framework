#include "DepthStencilPassTextureJsonConverter.h"

void FWK::Converter::DepthStencilPassTextureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DepthStencilPassTexture& a_depthStencilPassTexture) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_format = a_rootJson.value(k_formatJsonKey, Graphics::DepthStencilTexture::k_defaultDepthStencilTextureFormat);

	const auto l_depthClearValue = a_rootJson.value(k_depthClearValueJsonKey, Constant::k_defaultDepthClearValue);

	const auto l_stencilClearValue = a_rootJson.value(k_stencilClearValueJsonKey, Constant::k_defaultStencilClearValue);

	const auto l_renderGraphDepthStencilType = a_rootJson.value(k_renderGraphDepthStencilTypeJsonKey, Enum::RenderGraphDepthStencilType::Invalid);

	const auto l_width  = a_rootJson.value(k_widthJsonKey,  TextureBinaryConverter::k_emptyTextureWidth);
	const auto l_height = a_rootJson.value(k_heightJsonKey, TextureBinaryConverter::k_emptyTextureHeight);

	const auto l_isFixedSize = a_rootJson.value(k_isFixedSizeJsonKey, false);
	
	a_depthStencilPassTexture.SetFormat(l_format);

	a_depthStencilPassTexture.SetDepthClearValue(l_depthClearValue);

	a_depthStencilPassTexture.SetStencilClearValue(l_stencilClearValue);

	a_depthStencilPassTexture.SetRenderGraphDepthStencilType(l_renderGraphDepthStencilType);

	a_depthStencilPassTexture.SetWidth (l_width);
	a_depthStencilPassTexture.SetHeight(l_height);

	a_depthStencilPassTexture.SetIsFixedSize(l_isFixedSize);
}

nlohmann::json FWK::Converter::DepthStencilPassTextureJsonConverter::Serialize(const Graphics::DepthStencilPassTexture & a_depthStencilPassTexture) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_formatJsonKey]			             = a_depthStencilPassTexture.GetVALFormat		              ();
	l_rootJson[k_depthClearValueJsonKey]             = a_depthStencilPassTexture.GetVALDepthClearValue            ();
	l_rootJson[k_stencilClearValueJsonKey]           = a_depthStencilPassTexture.GetVALStencilClearValue          ();
	l_rootJson[k_renderGraphDepthStencilTypeJsonKey] = a_depthStencilPassTexture.GetVALRenderGraphDepthStencilType();
	l_rootJson[k_widthJsonKey]					     = a_depthStencilPassTexture.GetVALWidth					      ();
	l_rootJson[k_heightJsonKey]					     = a_depthStencilPassTexture.GetVALHeight			          ();
	l_rootJson[k_isFixedSizeJsonKey]			     = a_depthStencilPassTexture.GetVALIsFixedSize			      ();
	
	return l_rootJson;
}