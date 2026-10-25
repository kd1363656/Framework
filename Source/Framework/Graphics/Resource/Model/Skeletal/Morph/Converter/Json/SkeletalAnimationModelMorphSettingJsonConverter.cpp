#include "SkeletalAnimationModelMorphSettingJsonConverter.h"

void FWK::Converter::SkeletalAnimationModelMorphSettingJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting) const
{
    if (!Utility::IsJsonArray(a_rootJson, k_targetSettingListJsonKey)) { return; }

    const auto& l_targetSettingListJson = a_rootJson.at(k_targetSettingListJsonKey);

    std::vector<Struct::ModelMorphTargetSetting> l_targetSettingList = {};

    l_targetSettingList.reserve(l_targetSettingListJson.size());

    for (const auto& l_targetSettingJson : l_targetSettingListJson)
    {
        Struct::ModelMorphTargetSetting l_targetSetting = {};

        l_targetSetting.m_targetName      = Utility::DeserializeWString(l_targetSettingJson, k_targetNameJsonKey);
        l_targetSetting.m_lodProtection   = l_targetSettingJson.value  (k_lodProtectionJsonKey,   Enum::ModelMorphLODProtection::Normal);
        l_targetSetting.m_defaultWeight   = l_targetSettingJson.value  (k_defaultWeightJsonKey,   Constant::k_defaultModelMorphTargetWeight);
        l_targetSetting.m_minWeight       = l_targetSettingJson.value  (k_minWeightJsonKey,       Constant::k_defaultModelMorphTargetMINWeight);
        l_targetSetting.m_maxWeight       = l_targetSettingJson.value  (k_maxWeightJsonKey,       Constant::k_defaultModelMorphTargetMAXWeight);
        l_targetSetting.m_isShadowEnabled = l_targetSettingJson.value  (k_isShadowEnabledJsonKey, true);

        // 名前の無い設定は、どのターゲットの設定か分からないため読み込まない
        if (l_targetSetting.m_targetName.empty()) { continue; }

        l_targetSettingList.emplace_back(std::move(l_targetSetting));
    }

    a_morphSetting.SetTargetSettingList(std::move(l_targetSettingList));
}

nlohmann::json FWK::Converter::SkeletalAnimationModelMorphSettingJsonConverter::Serialize(const Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting) const
{
    const auto& l_targetSettingList = a_morphSetting.GetREFTargetSettingList();

    auto l_targetSettingListJson = nlohmann::json::array();

    for (const auto& l_targetSetting : l_targetSettingList)
    {
        nlohmann::json l_targetSettingJson = {};

        Utility::UpdateJson(l_targetSettingJson, Utility::SerializeWString(l_targetSetting.m_targetName, k_targetNameJsonKey));

        l_targetSettingJson[k_lodProtectionJsonKey]   = l_targetSetting.m_lodProtection;
        l_targetSettingJson[k_defaultWeightJsonKey]   = l_targetSetting.m_defaultWeight;
        l_targetSettingJson[k_minWeightJsonKey]       = l_targetSetting.m_minWeight;
        l_targetSettingJson[k_maxWeightJsonKey]       = l_targetSetting.m_maxWeight;
        l_targetSettingJson[k_isShadowEnabledJsonKey] = l_targetSetting.m_isShadowEnabled;

        l_targetSettingListJson.emplace_back(std::move(l_targetSettingJson));
    }

    nlohmann::json l_rootJson = {};

    l_rootJson[k_targetSettingListJsonKey] = std::move(l_targetSettingListJson);

    return l_rootJson;
}