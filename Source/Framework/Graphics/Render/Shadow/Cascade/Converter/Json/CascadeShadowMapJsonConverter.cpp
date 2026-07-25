#include "CascadeShadowMapJsonConverter.h"

void FWK::Converter::CascadeShadowMapJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::CascadeShadowMap& a_cascadeShadowMap) const
{

}

nlohmann::json FWK::Converter::CascadeShadowMapJsonConverter::Serialize(const Graphics::CascadeShadowMap & a_cascadeShadowMap) const
{
	return nlohmann::json();
}