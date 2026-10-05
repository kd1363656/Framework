#include "LightSystemJsonConverrter.h"

void FWK::Converter::LightSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::LightSystem& a_lightSystem) const
{
    auto& l_cbLightPass = a_lightSystem.GetMutableREFCbLightPass();

    if (!l_cbLightPass) 
    {
        l_cbLightPass = std::make_shared<Struct::CBLightPass>();
    }

    // ディレクショナルライトをデシリアライズ
    if (const auto& l_directionalLightJson = a_rootJson.value(k_directionalLightJsonKey, nlohmann::json{});
        !l_directionalLightJson.is_null())
    {
        auto& l_directionalLight = l_cbLightPass->m_directionalLight;

        l_directionalLight.m_direction = Utility::DeserializeVector3 (l_directionalLightJson, k_directionJsonKey);
        l_directionalLight.m_color     = Utility::DeserializeVector3 (l_directionalLightJson, k_colorJsonKey);
        l_directionalLight.m_intensity = l_directionalLightJson.value(k_intensityJsonKey,     Constant::k_defaultDirectionalLightIntensity);

        // ディレクショナルライトには必ずベクトルを持たせる
        if (l_directionalLight.m_direction.LengthSquared() <= Constant::k_epsilon)
        {
            l_directionalLight.m_direction = Constant::k_defaultDirectionalLightDirection;
        }

        // ライト方向はシェーダー側で内積計算に使うため、長さ1に正規化しておく
        // 正規化していないと、方向ベクトルの長さによって明るさまで変わってしまうため
        l_directionalLight.m_direction.Normalize();
    }

    // アンビエントライトをデシリアライズ
    if (const auto& l_ambientLightJson = a_rootJson.value(k_ambientLightJsonKey, nlohmann::json{});
        !l_ambientLightJson.is_null())
    {
        auto& l_ambientLight = l_cbLightPass->m_ambientLight;

        l_ambientLight.m_color     = Utility::DeserializeVector3(l_ambientLightJson, k_colorJsonKey);
        l_ambientLight.m_intensity = l_ambientLightJson.value   (k_intensityJsonKey, Constant::k_defaultAmbientLightIntensity);
    }
}

nlohmann::json FWK::Converter::LightSystemJsonConverter::Serialize(const Graphics::LightSystem& a_lightSystem) const
{
          nlohmann::json l_rootJson    = {};
    const auto&          l_cbLightPass = a_lightSystem.GetREFCbLightPass();

    if (!l_cbLightPass) { return {}; }

    const auto& l_directionalLight = l_cbLightPass->m_directionalLight;
    const auto& l_ambientLight     = l_cbLightPass->m_ambientLight;

    nlohmann::json l_directionalLightJson = {};

    // ディレクショナルライトをシリアライズ
    Utility::UpdateJson(l_directionalLightJson, Utility::SerializeVector3(l_directionalLight.m_color, k_colorJsonKey));
    Utility::UpdateJson(l_directionalLightJson, Utility::SerializeVector3(l_directionalLight.m_direction, k_directionJsonKey));

    l_directionalLightJson[k_intensityJsonKey] = l_directionalLight.m_intensity;

    l_rootJson[k_directionalLightJsonKey] = l_directionalLightJson;

    // アンビエントライトをシリアライズ
    nlohmann::json l_ambientLightJson = {};

    Utility::UpdateJson(l_ambientLightJson, Utility::SerializeVector3(l_ambientLight.m_color, k_colorJsonKey));

    l_ambientLightJson[k_intensityJsonKey] = l_ambientLight.m_intensity;

    l_rootJson[k_ambientLightJsonKey] = l_ambientLightJson;

    return l_rootJson;
}