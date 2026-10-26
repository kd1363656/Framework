#include "SkeletalAnimationModelSpringBoneSettingJsonConverter.h"

void FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const
{
    std::vector<Struct::ModelSpringBoneChainSetting>    l_chainSettingList    = {};
    std::vector<Struct::ModelSpringBoneColliderSetting> l_colliderSettingList = {};

    if (Utility::IsJsonArray(a_rootJson, k_chainSettingListJsonKey))
    {
        const auto& l_chainSettingListJson = a_rootJson.at(k_chainSettingListJsonKey);

        l_chainSettingList.reserve(l_chainSettingListJson.size());

        for (const auto& l_chainSettingJson : l_chainSettingListJson)
        {
            l_chainSettingList.emplace_back(DeserializeChainSetting(l_chainSettingJson));
        }
    }

    if (Utility::IsJsonArray(a_rootJson, k_colliderSettingListJsonKey))
    {
        const auto& l_colliderSettingListJson = a_rootJson.at(k_colliderSettingListJsonKey);

        l_colliderSettingList.reserve(l_colliderSettingListJson.size());

        for (const auto& l_colliderSettingJson : l_colliderSettingListJson)
        {
            l_colliderSettingList.emplace_back(DeserializeColliderSetting(l_colliderSettingJson));
        }
    }

    a_springBoneSetting.SetChainSettingList     (std::move(l_chainSettingList));
    a_springBoneSetting.SetColliderSettingList  (std::move(l_colliderSettingList));
    a_springBoneSetting.SetPrewarmIterationCount(a_rootJson.value(k_prewarmIterationCountJsonKey, Constant::k_defaultModelSpringBonePrewarmIterationCount));
}

FWK::Struct::ModelSpringBoneChainSetting FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::DeserializeChainSetting(const nlohmann::json& a_chainSettingJson) const
{
    Struct::ModelSpringBoneChainSetting l_chainSetting = {};

    if (Utility::IsJsonArray(a_chainSettingJson, k_rootBoneNameListJsonKey))
    {
        const auto& l_rootBoneNameListJson = a_chainSettingJson.at(k_rootBoneNameListJsonKey);

        l_chainSetting.m_rootBoneNameList.reserve(l_rootBoneNameListJson.size());

        // jsonの文字列はUTF-8のため、ボーンの名前(wstring)へ変換する
        for (const auto& l_rootBoneNameJson : l_rootBoneNameListJson)
        {
            l_chainSetting.m_rootBoneNameList.emplace_back(Utility::StringToWString(l_rootBoneNameJson.get<std::string>()));
        }
    }

    l_chainSetting.m_gravityDirection         = DeserializeVector3OrDefault(a_chainSettingJson, k_gravityDirectionJsonKey, Constant::k_defaultModelSpringBoneGravityDirection);
    l_chainSetting.m_stiffness                = a_chainSettingJson.value   (k_stiffnessJsonKey,                Constant::k_defaultModelSpringBoneStiffness);
    l_chainSetting.m_drag                     = a_chainSettingJson.value   (k_dragJsonKey,                     Constant::k_defaultModelSpringBoneDrag);
    l_chainSetting.m_gravityPower             = a_chainSettingJson.value   (k_gravityPowerJsonKey,             Constant::k_defaultModelSpringBoneGravityPower);
    l_chainSetting.m_hitRadius                = a_chainSettingJson.value   (k_hitRadiusJsonKey,                Constant::k_defaultModelSpringBoneHitRadius);
    l_chainSetting.m_maxAngularVelocityDegree = a_chainSettingJson.value   (k_maxAngularVelocityDegreeJsonKey, Constant::k_defaultModelSpringBoneMAXAngularVelocityDegree);
    l_chainSetting.m_endLength                = a_chainSettingJson.value   (k_endLengthJsonKey,                Constant::k_defaultModelSpringBoneEndLength);
    l_chainSetting.m_adjacentLinkStiffness    = a_chainSettingJson.value   (k_adjacentLinkStiffnessJsonKey,    Constant::k_defaultModelSpringBoneAdjacentLinkStiffness);
    l_chainSetting.m_isEnabled                = a_chainSettingJson.value   (k_isEnabledJsonKey,                true);
    l_chainSetting.m_isAdjacentChainLinked    = a_chainSettingJson.value   (k_isAdjacentChainLinkedJsonKey,    false);
    l_chainSetting.m_isAdjacentChainLooped    = a_chainSettingJson.value   (k_isAdjacentChainLoopedJsonKey,    false);

    return l_chainSetting;
}
FWK::Struct::ModelSpringBoneColliderSetting FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::DeserializeColliderSetting(const nlohmann::json& a_colliderSettingJson) const
{
    Struct::ModelSpringBoneColliderSetting l_colliderSetting = {};

    l_colliderSetting.m_boneName   = Utility::DeserializeWString(a_colliderSettingJson, k_boneNameJsonKey);
    l_colliderSetting.m_offset     = DeserializeVector3OrDefault(a_colliderSettingJson, k_offsetJsonKey,     Constant::k_defaultModelSpringBoneColliderOffset);
    l_colliderSetting.m_tailOffset = DeserializeVector3OrDefault(a_colliderSettingJson, k_tailOffsetJsonKey, Constant::k_defaultModelSpringBoneColliderTailOffset);
    l_colliderSetting.m_shape      = a_colliderSettingJson.value(k_shapeJsonKey,  Enum::ModelSpringBoneColliderShape::Sphere);
    l_colliderSetting.m_radius     = a_colliderSettingJson.value(k_radiusJsonKey, Constant::k_defaultModelSpringBoneColliderRadius);

    return l_colliderSetting;
}

FWK::TypeAlias::Math::Vector3 FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::DeserializeVector3OrDefault(const nlohmann::json& a_json, const std::string_view a_key, const TypeAlias::Math::Vector3& a_defaultValue) const
{
    // Utility::DeserializeVector3はキーが無いと(0, 0, 0)を返すため、
    // 重力の向きのように(0, 0, 0)が使えない値は、キーが無い場合に既定値を使う
    if (!a_json.contains(a_key.data())) { return a_defaultValue; }

    return Utility::DeserializeVector3(a_json, a_key);
}

nlohmann::json FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::Serialize(const Graphics::SkeletalAnimationModelSpringBoneSetting& a_springBoneSetting) const
{
    const auto& l_chainSettingList    = a_springBoneSetting.GetREFChainSettingList   ();
    const auto& l_colliderSettingList = a_springBoneSetting.GetREFColliderSettingList();

    auto l_chainSettingListJson    = nlohmann::json::array();
    auto l_colliderSettingListJson = nlohmann::json::array();

    for (const auto& l_chainSetting : l_chainSettingList)
    {
        l_chainSettingListJson.emplace_back(SerializeChainSetting(l_chainSetting));
    }

    for (const auto& l_colliderSetting : l_colliderSettingList)
    {
        l_colliderSettingListJson.emplace_back(SerializeColliderSetting(l_colliderSetting));
    }

    nlohmann::json l_rootJson = {};

    l_rootJson[k_chainSettingListJsonKey]      = std::move(l_chainSettingListJson);
    l_rootJson[k_colliderSettingListJsonKey]   = std::move(l_colliderSettingListJson);
    l_rootJson[k_prewarmIterationCountJsonKey] = a_springBoneSetting.GetVALPrewarmIterationCount();

    return l_rootJson;
}

nlohmann::json FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::SerializeChainSetting(const Struct::ModelSpringBoneChainSetting& a_chainSetting) const
{
    auto l_rootBoneNameListJson = nlohmann::json::array();

    for (const auto& l_rootBoneName : a_chainSetting.m_rootBoneNameList)
    {
        l_rootBoneNameListJson.emplace_back(Utility::WStringToString(l_rootBoneName));
    }

    nlohmann::json l_chainSettingJson = {};

    Utility::UpdateJson(l_chainSettingJson, Utility::SerializeVector3(a_chainSetting.m_gravityDirection, k_gravityDirectionJsonKey));

    l_chainSettingJson[k_rootBoneNameListJsonKey]         = std::move(l_rootBoneNameListJson);
    l_chainSettingJson[k_stiffnessJsonKey]                = a_chainSetting.m_stiffness;
    l_chainSettingJson[k_dragJsonKey]                     = a_chainSetting.m_drag;
    l_chainSettingJson[k_gravityPowerJsonKey]             = a_chainSetting.m_gravityPower;
    l_chainSettingJson[k_hitRadiusJsonKey]                = a_chainSetting.m_hitRadius;
    l_chainSettingJson[k_maxAngularVelocityDegreeJsonKey] = a_chainSetting.m_maxAngularVelocityDegree;
    l_chainSettingJson[k_endLengthJsonKey]                = a_chainSetting.m_endLength;
    l_chainSettingJson[k_adjacentLinkStiffnessJsonKey]    = a_chainSetting.m_adjacentLinkStiffness;
    l_chainSettingJson[k_isEnabledJsonKey]                = a_chainSetting.m_isEnabled;
    l_chainSettingJson[k_isAdjacentChainLinkedJsonKey]    = a_chainSetting.m_isAdjacentChainLinked;
    l_chainSettingJson[k_isAdjacentChainLoopedJsonKey]    = a_chainSetting.m_isAdjacentChainLooped;

    return l_chainSettingJson;
}
nlohmann::json FWK::Converter::SkeletalAnimationModelSpringBoneSettingJsonConverter::SerializeColliderSetting(const Struct::ModelSpringBoneColliderSetting& a_colliderSetting) const
{
    nlohmann::json l_colliderSettingJson = {};

    Utility::UpdateJson(l_colliderSettingJson, Utility::SerializeWString(a_colliderSetting.m_boneName,   k_boneNameJsonKey));
    Utility::UpdateJson(l_colliderSettingJson, Utility::SerializeVector3(a_colliderSetting.m_offset,     k_offsetJsonKey));
    Utility::UpdateJson(l_colliderSettingJson, Utility::SerializeVector3(a_colliderSetting.m_tailOffset, k_tailOffsetJsonKey));

    l_colliderSettingJson[k_shapeJsonKey]  = a_colliderSetting.m_shape;
    l_colliderSettingJson[k_radiusJsonKey] = a_colliderSetting.m_radius;

    return l_colliderSettingJson;
}