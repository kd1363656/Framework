#include "SkeletalAnimationModelSpringBoneSettingEditCommand.h"

FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditCommand::SkeletalAnimationModelSpringBoneSettingEditCommand(const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord, nlohmann::json&& a_beforeJson, nlohmann::json&& a_afterJson) :
    m_ownerRecord(a_ownerRecord),
    m_beforeJson (std::move(a_beforeJson)),
    m_afterJson  (std::move(a_afterJson))
{}
FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditCommand::~SkeletalAnimationModelSpringBoneSettingEditCommand() = default;

void FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditCommand::Undo()
{
    Apply(m_beforeJson);
}
void FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditCommand::Redo()
{
    Apply(m_afterJson);
}

void FWK::Graphics::SkeletalAnimationModelSpringBoneSettingEditCommand::Apply(const nlohmann::json& a_settingJson) const
{
    const auto& l_ownerRecord = m_ownerRecord.lock();

    // モデルが既に破棄されている場合は、戻す先が無いため何もしない
    if (!l_ownerRecord) { return; }

    auto& l_springBoneSetting = l_ownerRecord->GetMutableREFSpringBoneSetting();

    // Deserializeは設定の番号を進めるため、揺らす計算側もチェーンを作り直す
    l_springBoneSetting.Deserialize(a_settingJson);
}