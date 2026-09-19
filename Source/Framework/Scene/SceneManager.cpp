#include "SceneManager.h"

// アプリケーション起動時の初回はファイルパスに依存したファイル読み込みになるが
// 次回からはUUIDを通したFilePathの取得になる
void FWK::SceneManager::Load(const std::filesystem::path& a_nextSceneLoadFilePath)
{
    // ロード前に初期化を行う
    // (そのシーンで使用するSceneShiftMapなどの情報を消して、次のシーンでしか使用しない情報に置き換えるため)
    INIT();

    // 現在読み込んだシーンのファイルパスを格納する
    m_currentSceneFilePath = a_nextSceneLoadFilePath;

    m_jsonConverter.Load(*this);

    // Sceneの作成に失敗していればログで出力する
    if (!m_scene)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Sceneが無効なためSceneのロードに失敗しました。");

        return;
    }

    // シーンをロードしてデシリアライズした後の処理
    m_scene->PostDeserialize();
}

void FWK::SceneManager::EarlyUpdate()
{
    if (!m_scene) { return; }

    m_scene->EarlyUpdate();
}
void FWK::SceneManager::Update() const
{
    if (!m_scene) { return; }

    m_scene->Update ();
}
void FWK::SceneManager::LateUpdate() const
{
    if (!m_scene) { return; }

    m_scene->LateUpdate();
}
void FWK::SceneManager::PostLateUpdate()
{
    if (!m_scene) { return; }

    m_scene->PostLateUpdate();

    // シーン内部で全ての処理が終わった後に
    // シーンを
    LoadNextSceneIfNeeded();
}

void FWK::SceneManager::Save() const
{
    m_jsonConverter.Save(*this);
}

void FWK::SceneManager::INIT()
{
    // 現在保持しているシーンをリセットして新しいシーンを作成
    m_scene = std::make_shared<Scene>();

    m_scene->INIT();

    m_currentSceneFilePath.clear();
}

void FWK::SceneManager::LoadNextSceneIfNeeded()
{
    if (!m_scene) { return; }

    const auto& l_nextSceneLoadFilePath = m_scene->FetchVALNextLoadSceneFilePath();

    // 毎フレーム確認してもしファイルのパスが空なら
    // ファイルパスが返されていないという意味なのでreturn;
    if (l_nextSceneLoadFilePath.empty()) { return; }

    // シーンマネージャーのシーン遷移情報をクリアして
    // シーン遷移情報及びシーンを読み込む
    Load(l_nextSceneLoadFilePath);
}