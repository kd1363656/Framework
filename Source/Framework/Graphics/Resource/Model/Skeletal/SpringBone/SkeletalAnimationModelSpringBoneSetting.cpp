#include "SkeletalAnimationModelSpringBoneSetting.h"

FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::SkeletalAnimationModelSpringBoneSetting(SkeletalAnimationModelSpringBoneSetting&& a_other) noexcept :
    m_chainSettingList   (std::move(a_other.m_chainSettingList)),
    m_colliderSettingList(std::move(a_other.m_colliderSettingList)),

    m_editInspector  (),
    m_binaryConverter(),
    m_jsonConverter  (),

    m_prewarmIterationCount(a_other.m_prewarmIterationCount),
    m_revision             (a_other.m_revision),

    m_isUnsaved(a_other.m_isUnsaved)
{}

FWK::Graphics::SkeletalAnimationModelSpringBoneSetting& FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::operator=(SkeletalAnimationModelSpringBoneSetting&& a_other) noexcept
{
    if (this == &a_other) { return *this; }

    m_chainSettingList    = std::move(a_other.m_chainSettingList);
    m_colliderSettingList = std::move(a_other.m_colliderSettingList);

    m_prewarmIterationCount = a_other.m_prewarmIterationCount;

    // 中身が入れ替わったため、揺らす計算側にチェーンを作り直させる
    ++m_revision;

    m_isUnsaved = a_other.m_isUnsaved;

    return *this;
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::LoadBinaryData(const std::filesystem::path& a_modelFilePath)
{
    if (!m_binaryConverter.Load(*this, a_modelFilePath)) { return false; }

    ++m_revision;

    // ファイルと同じ内容になったため、未保存ではない
    m_isUnsaved = false;

    return true;
}

void FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);

    MarkChanged();
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::EditInspector(const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord)
{
    return m_editInspector.EditInspector(*this, a_ownerRecord);
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::SaveBinaryData(const std::filesystem::path& a_modelFilePath)
{
    if (!m_binaryConverter.Save(*this, a_modelFilePath)) { return false; }

    m_isUnsaved = false;

    return true;
}

nlohmann::json FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::MarkChanged()
{
    ++m_revision;

    m_isUnsaved = true;
}

bool FWK::Graphics::SkeletalAnimationModelSpringBoneSetting::IsActive() const
{
    return std::ranges::any_of(m_chainSettingList, [](const Struct::ModelSpringBoneChainSetting& a_chainSetting)
    {
        return a_chainSetting.m_isEnabled &&
               !a_chainSetting.m_rootBoneNameList.empty();
    });
}