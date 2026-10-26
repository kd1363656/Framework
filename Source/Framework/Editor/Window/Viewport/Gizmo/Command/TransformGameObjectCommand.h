#pragma once

namespace FWK::Editor
{
    // ビューポートのギズモでGameObjectを動かした操作を、Undo / Redoで戻す / やり直すためのコマンド
    // GameObjectはUUIDで覚えておき、実行するたびにシーンから探す(再生 / 停止でシーンが作り直されても使えるようにするため)
    class TransformGameObjectCommand final : public ICommand
    {
    public:

         TransformGameObjectCommand(std::vector<Struct::TransformGameObjectRecord>&& a_recordList);
        ~TransformGameObjectCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        void ApplyTransform(const bool a_isBefore) const;

        static constexpr bool k_applyBeforeTransform = true;
        static constexpr bool k_applyAfterTransform  = false;

        std::vector<Struct::TransformGameObjectRecord> m_recordList;
    };
}
