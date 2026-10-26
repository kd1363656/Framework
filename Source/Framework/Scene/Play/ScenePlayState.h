#pragma once

namespace FWK
{
    // エディターの▶(再生)と■(停止)の状態
    // 再生を始めるときにシーン全体をjsonへ写しておき、停止したときにその内容へ戻す
    // (再生中にゲームオブジェクトが動いても、停止すれば再生前の状態に戻る)
    class ScenePlayState final
    {
    public:

         ScenePlayState() = default;
        ~ScenePlayState() = default;

        void BeginPlay(nlohmann::json&& a_snapshotJson, const std::filesystem::path& a_snapshotSceneFilePath);
        void EndPlay  ();

        const auto& GetREFSnapshotJson         () const { return m_snapshotJson; }
        const auto& GetREFSnapshotSceneFilePath() const { return m_snapshotSceneFilePath; }

        bool GetVALIsPlaying() const { return m_isPlaying; }

    private:

        nlohmann::json m_snapshotJson = {};

        std::filesystem::path m_snapshotSceneFilePath = {};

        bool m_isPlaying = false;
    };
}
