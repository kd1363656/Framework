#include "SkeletalAnimationModelMorphSetting.h"

FWK::Graphics::SkeletalAnimationModelMorphSetting::SkeletalAnimationModelMorphSetting(SkeletalAnimationModelMorphSetting&& a_other) noexcept :
    m_targetSettingList(std::move(a_other.m_targetSettingList)),

    m_editInspector  (),
    m_binaryConverter(),
    m_jsonConverter  ()
{}

FWK::Graphics::SkeletalAnimationModelMorphSetting& FWK::Graphics::SkeletalAnimationModelMorphSetting::operator=(SkeletalAnimationModelMorphSetting&& a_other) noexcept
{
    if (this == &a_other) { return *this; }

    m_targetSettingList = std::move(a_other.m_targetSettingList);

    return *this;
}

bool FWK::Graphics::SkeletalAnimationModelMorphSetting::LoadBinaryData(const std::filesystem::path& a_modelFilePath)
{
    return m_binaryConverter.Load(*this, a_modelFilePath);
}

void FWK::Graphics::SkeletalAnimationModelMorphSetting::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Graphics::SkeletalAnimationModelMorphSetting::EditInspector()
{
    return m_editInspector.EditInspector(*this);
}

bool FWK::Graphics::SkeletalAnimationModelMorphSetting::AddMissingTargetSetting(const std::vector<Struct::ModelMorphTarget>& a_targetList)
{
    bool l_isAdded = false;

    for (const auto& l_target : a_targetList)
    {
        if (FindPTRTargetSetting(l_target.m_targetName)) { continue; }

        Struct::ModelMorphTargetSetting l_targetSetting = {};

        l_targetSetting.m_targetName = l_target.m_targetName;

        m_targetSettingList.emplace_back(std::move(l_targetSetting));

        l_isAdded = true;
    }

    return l_isAdded;
}
void FWK::Graphics::SkeletalAnimationModelMorphSetting::ApplyLODProtection(std::vector<Struct::ModelMorphTarget>& a_targetList) const
{
    for (auto& l_target : a_targetList)
    {
        l_target.m_lodProtection = FetchVALLODProtection(l_target.m_targetName);
    }
}

bool FWK::Graphics::SkeletalAnimationModelMorphSetting::SaveBinaryData(const std::filesystem::path& a_modelFilePath)
{
    return m_binaryConverter.Save(*this, a_modelFilePath);
}

nlohmann::json FWK::Graphics::SkeletalAnimationModelMorphSetting::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

bool FWK::Graphics::SkeletalAnimationModelMorphSetting::IsLODProtectionApplied(const std::vector<Struct::ModelMorphTarget>& a_targetList) const
{
    return std::ranges::all_of(a_targetList, [this](const Struct::ModelMorphTarget& a_target)
    {
        return a_target.m_lodProtection == FetchVALLODProtection(a_target.m_targetName);
    });
}

const FWK::Struct::ModelMorphTargetSetting* FWK::Graphics::SkeletalAnimationModelMorphSetting::FindPTRTargetSetting(const std::wstring& a_targetName) const
{
    const auto& l_itr = std::ranges::find(m_targetSettingList, a_targetName, &Struct::ModelMorphTargetSetting::m_targetName);

    if (l_itr == m_targetSettingList.end()) { return nullptr; }

    return &(*l_itr);
}

FWK::Enum::ModelMorphLODProtection FWK::Graphics::SkeletalAnimationModelMorphSetting::FetchVALLODProtection(const std::wstring& a_targetName) const
{
    const auto* l_targetSetting = FindPTRTargetSetting(a_targetName);

    // 設定が無いターゲットは、通常の扱いにする
    if (!l_targetSetting) { return Enum::ModelMorphLODProtection::Normal; }

    return l_targetSetting->m_lodProtection;
}