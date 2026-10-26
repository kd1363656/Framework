#pragma once

namespace FWK::Graphics
{
    // エディターで揺れものの設定を変えたときの、Undo/Redo1回分
    // 編集前と編集後の設定をjsonで丸ごと覚えておき、Undoで編集前、Redoで編集後の設定へ戻す
    // (設定は小さいため、どの値が変わったかを細かく覚えるより、丸ごと覚える方が簡単で間違えにくい)
    class SkeletalAnimationModelSpringBoneSettingEditCommand final : public ICommand
    {
    public:

         SkeletalAnimationModelSpringBoneSettingEditCommand(const std::weak_ptr<SkeletalAnimationModelRecord>& a_ownerRecord, nlohmann::json&& a_beforeJson, nlohmann::json&& a_afterJson);
        ~SkeletalAnimationModelSpringBoneSettingEditCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void Apply(const nlohmann::json& a_settingJson) const;

        // モデルが破棄された後にUndoされても壊れないように、weak_ptrで持つ
        std::weak_ptr<SkeletalAnimationModelRecord> m_ownerRecord;

        nlohmann::json m_beforeJson;
        nlohmann::json m_afterJson;
    };
}