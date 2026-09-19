#pragma once

namespace FWK
{
    class SceneManager final : public Utility::SingletonBase<SceneManager>
    {
    private:

        friend class SingletonBase<SceneManager>;

         SceneManager()          = default;
        ~SceneManager() override = default;

    public:

        void Load(const std::filesystem::path& a_nextSceneLoadFilePath);

        void EarlyUpdate   ();
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate();

        void Save() const;

        void SetCurrentSceneFilePath(const std::filesystem::path& a_set) { m_currentSceneFilePath = a_set; }

        const auto& GetREFCurrentSceneFilePath() const { return m_currentSceneFilePath; }

        std::weak_ptr<Scene> GetVALScene() const { return m_scene; }

    private:

        void INIT();

        void LoadNextSceneIfNeeded();

        std::shared_ptr<Scene> m_scene = nullptr;

        std::filesystem::path m_currentSceneFilePath = {};

        Converter::SceneManagerJsonConverter m_jsonConverter = {};
    };
}