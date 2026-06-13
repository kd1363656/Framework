#include "RenderTargetPassTextureJsonConverter.h"

void FWK::Converter::RenderTargetPassTextureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RenderTargetPassTexture& a_renderTargetPassTexture) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_clearColor = Utility::DeserializeColor(a_rootJson, k_clearColorJsonKey);

	const auto l_format = a_rootJson.value(k_formatJsonKey, Constant::k_defaultRenderTargetTextureFormat);

	const auto l_renderGraphResourceType = a_rootJson.value(k_renderGraphResourceTypeJsonKey, Enum::RenderGraphResourceType::Invalid);

	const auto l_width  = a_rootJson.value(k_widthJsonKey,  Constant::k_emptyTextureWidth);
	const auto l_height = a_rootJson.value(k_heightJsonKey, Constant::k_emptyTextureHeight);

	const auto l_isFixedSize = a_rootJson.value(k_isFixedSizeJsonKey, false);

	a_renderTargetPassTexture.SetClearColor(l_clearColor);

	a_renderTargetPassTexture.SetFormat(l_format);

	a_renderTargetPassTexture.SetRenderGraphResourceType(l_renderGraphResourceType);

	a_renderTargetPassTexture.SetWidth (l_width);
	a_renderTargetPassTexture.SetHeight(l_height);

	a_renderTargetPassTexture.SetIsFixedSize(l_isFixedSize);
}

nlohmann::json FWK::Converter::RenderTargetPassTextureJsonConverter::Serialize(const Graphics::RenderTargetPassTexture & a_renderTargetPassTexture) const
{
	nlohmann::json l_rootJson = {};

	Utility::UpdateJson(l_rootJson, Utility::SerializeColor(a_renderTargetPassTexture.GetREFClearColor(), k_clearColorJsonKey));

	l_rootJson[k_formatJsonKey]					 = a_renderTargetPassTexture.GetVALFormat				  ();
	l_rootJson[k_renderGraphResourceTypeJsonKey] = a_renderTargetPassTexture.GetVALRenderGraphResourceType();
	l_rootJson[k_widthJsonKey]					 = a_renderTargetPassTexture.GetVALWidth					  ();
	l_rootJson[k_heightJsonKey]					 = a_renderTargetPassTexture.GetVALHeight				  ();
	l_rootJson[k_isFixedSizeJsonKey]			 = a_renderTargetPassTexture.GetVALIsFixedSize			  ();

	return l_rootJson;
}