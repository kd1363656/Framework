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

        bool Load(const std::filesystem::path& a_nextSceneLoadFilePath);

        void EarlyUpdate          ();
        void Update               () const;
        void LateUpdate           () const;
        void PostLateUpdate       ();
        bool LoadNextSceneIfNeeded();

        // エディターの▶ : シーンを写し取ってから、GameObjectのUpdate / LateUpdateを動かし始める
        // エディターの■ : 写し取っておいた内容へシーンを戻し、Update / LateUpdateを止める
        // (停止中もEarlyUpdate / PostLateUpdateは動かし、追加・削除やTransformの行列更新はエディターで使えるようにする)
        bool BeginPlay();
        bool EndPlay  ();

        // シーンの新規作成・読み込みの要求
        // エディターのUI描画中にシーンを作り直すと、描画中のウィンドウが古いシーンを参照したままになるため
        // その場では作り直さず、要求だけを覚えておき、LoadNextSceneIfNeededでまとめて処理する
        bool RequestNewScene ();
        bool RequestLoadScene(const std::filesystem::path& a_sceneFilePath);

        bool Save  ()                                                                                              const;
        bool SaveAs(const std::filesystem::path& a_sceneFilePath, AssetFilePathRegistry& a_assetFilePathRegistry);

        void SetCurrentSceneFilePath(const std::filesystem::path& a_set) { m_currentSceneFilePath = a_set; }

        const auto& GetREFCurrentSceneFilePath() const { return m_currentSceneFilePath; }

        std::weak_ptr<Scene> GetVALScene() const { return m_scene; }

        bool FetchVALIsPlaying() const { return m_playState.GetVALIsPlaying(); }

    private:

        void INIT();

        void ProcessSceneLoadRequest();

        std::shared_ptr<Scene> m_scene = nullptr;

        ScenePlayState m_playState = {};

        std::filesystem::path m_currentSceneFilePath = {};

        // 空のときは「新規シーンの作成」の要求として扱う
        std::filesystem::path m_requestedLoadSceneFilePath = {};

        bool m_hasSceneLoadRequest = false;

        Converter::SceneManagerJsonConverter m_jsonConverter = {};
    };
}