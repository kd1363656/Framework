#include "DepthStencilPassTextureJsonConverter.h"

void FWK::Converter::DepthStencilPassTextureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::DepthStencilPassTexture& a_depthStencilPassTexture) const
{
	if (a_rootJson.is_null()) { return; }

	const auto l_format = a_rootJson.value(k_formatJsonKey, Constant::k_defaultDepthStencilTextureFormat);

	const auto l_depthClearValue = a_rootJson.value(k_depthClearValueJsonKey, Constant::k_defaultDepthClearValue);

	const auto l_staticClearValue = a_rootJson.value(k_stencilClearValueJsonKey, Constant::k_defaultStencilClearValue);

	const auto l_renderGraphResourceType = a_rootJson.value(k_renderGraphResourceTypeJsonKey, Enum::RenderGraphResourceType::Invalid);

	const auto l_width  = a_rootJson.value(k_widthJsonKey,  Constant::k_emptyTextureWidth);
	const auto l_height = a_rootJson.value(k_heightJsonKey, Constant::k_emptyTextureHeight);

	const auto l_isFixedSize			 = a_rootJson.value(k_isFixedSizeJsonKey,			  false);
	const auto l_isSkipClearOnBeginFrame = a_rootJson.value(k_isSkipClearOnBeginFrameJsonKey, false);

	a_depthStencilPassTexture.SetFormat(l_format);

	a_depthStencilPassTexture.SetDepthClearValue(l_depthClearValue);

	a_depthStencilPassTexture.SetStencilClearValue(l_staticClearValue);

	a_depthStencilPassTexture.SetRenderGraphResourceType(l_renderGraphResourceType);

	a_depthStencilPassTexture.SetWidth (l_width);
	a_depthStencilPassTexture.SetHeight(l_height);

	a_depthStencilPassTexture.SetIsFixedSize			(l_isFixedSize);
	a_depthStencilPassTexture.SetIsSkipClearOnBeginFrame(l_isSkipClearOnBeginFrame);
}

nlohmann::json FWK::Converter::DepthStencilPassTextureJsonConverter::Serialize(const Graphics::DepthStencilPassTexture & a_depthStencilPassTexture) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_formatJsonKey]			         = a_depthStencilPassTexture.GetVALFormat		          ();
	l_rootJson[k_depthClearValueJsonKey]         = a_depthStencilPassTexture.GetVALDepthClearValue        ();
	l_rootJson[k_stencilClearValueJsonKey]       = a_depthStencilPassTexture.GetVALStencilClearValue      ();
	l_rootJson[k_renderGraphResourceTypeJsonKey] = a_depthStencilPassTexture.GetVALRenderGraphResourceType();
	l_rootJson[k_widthJsonKey]					 = a_depthStencilPassTexture.GetVALWidth					  ();
	l_rootJson[k_isFixedSizeJsonKey]			 = a_depthStencilPassTexture.GetVALIsFixedSize			  ();
	l_rootJson[k_isSkipClearOnBeginFrameJsonKey] = a_depthStencilPassTexture.GetVALIsSkipClearOnBeginFrame();

	return l_rootJson;
}