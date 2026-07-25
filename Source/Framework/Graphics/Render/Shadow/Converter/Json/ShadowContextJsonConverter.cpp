#include "ShadowContextJsonConverter.h"

void FWK::Converter::ShadowContextJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ShadowContext& a_shadowContext) const
{

}

nlohmann::json FWK::Converter::ShadowContextJsonConverter::Serialize(const Graphics::ShadowContext& a_shadowContext) const
{
    return nlohmann::json();
}