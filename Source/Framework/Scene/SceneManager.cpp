#include "SceneManager.h"
#include "../../Application/Application.h"

// アプリケーション起動時の初回はファイルパスに依存したファイル読み込みになるが
// 次回からはUUIDを通したFilePathの取得になる
bool FWK::SceneManager::Load(const std::filesystem::path& a_nextSceneLoadFilePath)
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

        return false;
    }

    m_scene->PostDeserialize();

    return true;
}

void FWK::SceneManager::EarlyUpdate()
{
    if (!m_scene) { return; }

    m_scene->EarlyUpdate();
}
void FWK::SceneManager::Update() const
{
    if (!m_scene) { return; }

    // ゲームの処理は再生中(エディターの▶を押した後)だけ動かす
    if (!m_playState.GetVALIsPlaying()) { return; }

    m_scene->Update ();
}
void FWK::SceneManager::LateUpdate() const
{
    if (!m_scene) { return; }

    if (!m_playState.GetVALIsPlaying()) { return; }

    m_scene->LateUpdate();
}
void FWK::SceneManager::PostLateUpdate()
{
    if (!m_scene) { return; }

    m_scene->PostLateUpdate();
}
bool FWK::SceneManager::LoadNextSceneIfNeeded()
{
    // エディターから新規作成・読み込みの要求が来ていれば、シーン遷移より優先して処理する
    // (trueを返すと、呼び出し側で前のシーンのUndo / Redo履歴が消される)
    if (m_hasSceneLoadRequest)
    {
        ProcessSceneLoadRequest();

        return true;
    }

    if (!m_scene) { return false; }

    const auto& l_nextSceneLoadFilePath = m_scene->FetchVALNextLoadSceneFilePath();
    
    // 毎フレーム確認してもしファイルのパスが空なら
    // ファイルパスが返されていないという意味なのでreturn;
    if (l_nextSceneLoadFilePath.empty()) { return false; }

    // シーンマネージャーのシーン遷移情報をクリアして
    // シーン遷移情報及びシーンを読み込む
    Load(l_nextSceneLoadFilePath);

    return true;
}

bool FWK::SceneManager::BeginPlay()
{
    if (!m_scene ||
        m_playState.GetVALIsPlaying())
    {
        return false;
    }

    // 停止したときに戻せるよう、再生前のシーン全体をjsonへ写しておく
    // (ファイルには保存しないため、再生中に変わった内容がシーンファイルへ残ることはない)
    m_playState.BeginPlay(m_scene->Serialize(), m_currentSceneFilePath);

    return true;
}
bool FWK::SceneManager::EndPlay()
{
    if (!m_playState.GetVALIsPlaying()) { return false; }

    // 写しておいたjsonから、シーンを読み込み直す(ファイルから読むLoadと同じ手順)
    // INIT()はシーンだけを作り直し、m_playStateには触れないため、参照のまま使える
    const auto& l_snapshotJson          = m_playState.GetREFSnapshotJson         ();
    const auto& l_snapshotSceneFilePath = m_playState.GetREFSnapshotSceneFilePath();

    const auto l_sceneFilePath = l_snapshotSceneFilePath;

    INIT();

    m_currentSceneFilePath = l_sceneFilePath;

    m_scene->Deserialize    (l_snapshotJson);
    m_scene->PostDeserialize();

    m_playState.EndPlay();

    return true;
}

bool FWK::SceneManager::RequestNewScene()
{
    // 再生中に作り直すと、■を押したときに戻すはずの再生前のシーンが分からなくなるため受け付けない
    if (m_playState.GetVALIsPlaying())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "再生中はシーンを新規作成できません。■で停止してから実行してください。");

        return false;
    }

    // 読み込むファイルパスを空にしておくことで「新規作成」の要求になる
    m_requestedLoadSceneFilePath.clear();

    m_hasSceneLoadRequest = true;

    return true;
}
bool FWK::SceneManager::RequestLoadScene(const std::filesystem::path& a_sceneFilePath)
{
    if (m_playState.GetVALIsPlaying())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "再生中はシーンを読み込めません。■で停止してから実行してください。");

        return false;
    }

    // シーンとして登録されていないJson(Prefabなど)をシーンとして読み込むと中身が壊れるため
    // AssetFilePathRegistryにシーンとして登録されているファイルだけを受け付ける
    const auto& l_application           = Application::GetInstance                 ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry();

    if (!l_assetFilePathRegistry.ContainsAssetFilePath(a_sceneFilePath, Enum::AssetFilePathRegistryType::Scene))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーンとして登録されていないファイルのため、読み込みませんでした。\nFilePath : {}", a_sceneFilePath.string());

        return false;
    }

    m_requestedLoadSceneFilePath = a_sceneFilePath;
    m_hasSceneLoadRequest        = true;

    return true;
}

bool FWK::SceneManager::Save() const
{
    // 再生中のシーンは▶を押した後に動いた状態なので、ファイルへ書き込まない
    if (m_playState.GetVALIsPlaying())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "再生中はシーンを保存できません。■で停止してから実行してください。");

        return false;
    }

    // 新規作成したシーンはまだ保存先が決まっていないため、呼び出し側でSaveAsを使ってもらう
    if (m_currentSceneFilePath.empty()) { return false; }

    if (!m_jsonConverter.Save(*this))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーンの保存に失敗しました。\nFilePath : {}", m_currentSceneFilePath.string());

        return false;
    }

    return true;
}
bool FWK::SceneManager::SaveAs(const std::filesystem::path& a_sceneFilePath, AssetFilePathRegistry& a_assetFilePathRegistry)
{
    if (!m_scene) { return false; }

    if (m_playState.GetVALIsPlaying())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "再生中はシーンを保存できません。■で停止してから実行してください。");

        return false;
    }

    // AssetBrowserのWatcherが見ているのはAssetフォルダの中だけなので、それ以外の場所には保存しない
    // (Assetフォルダの外に保存すると、AssetFilePathRegistryでシーンとして管理できなくなる)
    if (a_sceneFilePath.extension() != Constant::k_lowerJsonExtension ||
        !Utility::IsFilePathUnderRootDirectory(a_sceneFilePath, Constant::k_assetRootFolderPath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーンはAssetフォルダの中に.jsonとして保存してください。\nFilePath : {}", a_sceneFilePath.string());

        return false;
    }

    // 既にシーン以外(Prefabなど)として登録されているファイルは上書きしない
    const bool l_isRegisteredScene = a_assetFilePathRegistry.ContainsAssetFilePath(a_sceneFilePath, Enum::AssetFilePathRegistryType::Scene);

    if (!l_isRegisteredScene &&
        a_assetFilePathRegistry.FindPTRAssetUUID(a_sceneFilePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーン以外のアセットとして登録されているファイルのため、上書きしませんでした。\nFilePath : {}", a_sceneFilePath.string());

        return false;
    }

    // まだ登録されていない新しいファイルなら、書き込む前にシーンとして登録する
    // WatcherはRegistryに登録されていないJsonがAssetフォルダに現れると削除してしまうため、必ずファイルより先に登録する
    if (!l_isRegisteredScene)
    {
              auto& l_uuidManager = Utility::UUIDManager::GetInstance();
        const auto& l_sceneUUID   = l_uuidManager.GenerateVALUUID    ();

        if (!a_assetFilePathRegistry.Add(a_sceneFilePath, l_sceneUUID, Enum::AssetFilePathRegistryType::Scene))
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "AssetFilePathRegistryへのScene登録に失敗したため、シーンを保存しませんでした。\nFilePath : {}", a_sceneFilePath.string());

            return false;
        }
    }

    // 失敗したときに元へ戻せるよう、変更前の名前とファイルパスを覚えておく
    const auto l_beforeSceneName     = m_scene->GetREFName();
    const auto l_beforeSceneFilePath = m_currentSceneFilePath;

    // AssetBrowserでのリネームと同じく、シーン名はファイル名(拡張子なし)にそろえる
    // 例 : Asset/Data/Scene/Stage1.json -> Stage1
    m_scene->SetName(a_sceneFilePath.stem().string());

    // 現在のシーンのファイルパスを先に切り替えておく
    // WatcherはCurrentSceneのファイルパスと同じファイルの追加通知を「自分の保存」として無視するため
    m_currentSceneFilePath = a_sceneFilePath;

    if (!m_jsonConverter.Save(*this))
    {
        m_scene->SetName(l_beforeSceneName);

        m_currentSceneFilePath = l_beforeSceneFilePath;

        // 今回新しく登録した分だけ登録前の状態へ戻す
        if (!l_isRegisteredScene)
        {
            a_assetFilePathRegistry.Erase(a_sceneFilePath);
        }

        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "シーンの保存に失敗したため、保存先の変更を取り消しました。\nFilePath : {}", a_sceneFilePath.string());

        return false;
    }

    return true;
}

void FWK::SceneManager::INIT()
{
    // 現在保持しているシーンをリセットして新しいシーンを作成
    m_scene = std::make_shared<Scene>();

    m_scene->INIT();

    m_currentSceneFilePath.clear();
}

void FWK::SceneManager::ProcessSceneLoadRequest()
{
    // 要求は1回処理したら消す(Loadが失敗しても毎フレーム読み込み直さないようにするため)
    const auto l_requestedLoadSceneFilePath = m_requestedLoadSceneFilePath;

    m_requestedLoadSceneFilePath.clear();

    m_hasSceneLoadRequest = false;

    // ファイルパスが空なら新規作成
    // INITで空のシーンを作り、保存先は未定(空)のままにする(最初の保存で「名前を付けて保存」になる)
    if (l_requestedLoadSceneFilePath.empty())
    {
        INIT();

        FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "新しいシーンを作成しました。");

        return;
    }

    if (!Load(l_requestedLoadSceneFilePath)) { return; }

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "シーンを読み込みました。\nFilePath : {}", l_requestedLoadSceneFilePath.string());
}