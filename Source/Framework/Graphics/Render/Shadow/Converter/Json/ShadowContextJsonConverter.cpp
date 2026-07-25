#include "ShadowContextJsonConverter.h"

void FWK::Converter::ShadowContextJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::ShadowContext& a_shadowContext) const
{
    if (a_rootJson.is_null()) { return; }

    if (const auto& l_json = a_rootJson.value(k_cascadeShadowMapJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        auto& l_cascadeShadowMap = a_shadowContext.GetMutableREFCascadeShadowMap();

        l_cascadeShadowMap.Deserialize(l_json);
    }
}

nlohmann::json FWK::Converter::ShadowContextJsonConverter::Serialize(const Graphics::ShadowContext& a_shadowContext) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_cascadeShadowMap = a_shadowContext.GetREFCascadeShadowMap();

    l_rootJson[k_cascadeShadowMapJsonKey] = l_cascadeShadowMap.Serialize();

    return l_rootJson;
}