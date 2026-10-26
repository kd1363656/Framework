#include "FileMainMenuEditor.h"
#include "../../../../Application/Application.h"

void FWK::Editor::FileMainMenuEditor::Draw()
{
    DrawMenuFile             ();
    UpdateShortCutKey        ();
    ApplyFileDialogResultList();
}

void FWK::Editor::FileMainMenuEditor::DrawMenuFile()
{
    if (!ImGui::BeginMenu(k_beginFileMenuTextString.data())) { return; }

    // 再生中やダイアログを表示している間は押せないようにする(灰色で表示される)
    const bool l_canOperateScene = CanOperateScene();

    // ImGui::MenuItem(表示名, ショートカットの表示, チェックマークを付けるか, 押せるか)
    // ショートカットの表示は文字を出すだけで、実際のキー入力はUpdateShortCutKeyで判定する
    if (ImGui::MenuItem(k_newSceneTextString.data(),
                        k_newSceneShortCutString.data(),
                        k_isMenuItemNotSelected,
                        l_canOperateScene))
    {
        NewScene();
    }

    if (ImGui::MenuItem(k_openSceneTextString.data(),
                        k_openSceneShortCutString.data(),
                        k_isMenuItemNotSelected,
                        l_canOperateScene))
    {
        OpenScene();
    }

    ImGui::Separator();

    if (ImGui::MenuItem(k_saveTextString.data(),
                        k_saveShortCutString.data(),
                        k_isMenuItemNotSelected,
                        l_canOperateScene))
    {
        SaveScene();
    }

    if (ImGui::MenuItem(k_saveAsTextString.data(),
                        k_saveAsShortCutString.data(),
                        k_isMenuItemNotSelected,
                        l_canOperateScene))
    {
        SaveSceneAs();
    }

    ImGui::EndMenu();
}
void FWK::Editor::FileMainMenuEditor::UpdateShortCutKey()
{
    // リネーム欄などで文字を入力している間は、Ctrl+Sなどをファイル操作として扱わない
    if (const auto& l_io = ImGui::GetIO();
        l_io.WantTextInput)
    {
        return;
    }

    if (!CanOperateScene()) { return; }

    // ImGui::IsKeyChordPressed : 修飾キー(Ctrl、Shift)が完全に一致した状態で、キーが押された瞬間だけtrueを返す
    // 完全一致なので、Ctrl+Shift+Sを押したときにCtrl+S(保存)が一緒に反応することはない
    if      (ImGui::IsKeyChordPressed(k_newSceneKeyChord))  { NewScene   (); }
    else if (ImGui::IsKeyChordPressed(k_openSceneKeyChord)) { OpenScene  (); }
    else if (ImGui::IsKeyChordPressed(k_saveKeyChord))      { SaveScene  (); }
    else if (ImGui::IsKeyChordPressed(k_saveAsKeyChord))    { SaveSceneAs(); }
}
void FWK::Editor::FileMainMenuEditor::ApplyFileDialogResultList()
{
    // ダイアログは別スレッドで表示しているため、閉じられたかどうかを毎フレーム確認する
    // キャンセルされた場合はTryFetchResultFilePathがfalseを返すので何もしない
    std::filesystem::path l_resultFilePath = {};

    if (m_openSceneFileDialog.TryFetchResultFilePath(l_resultFilePath))
    {
        auto& l_sceneManager = SceneManager::GetInstance();

        // 実際の読み込みは、このフレームのエディター描画が終わった後(次フレームの更新処理の後)に行われる
        l_sceneManager.RequestLoadScene(l_resultFilePath);
    }

    if (m_saveSceneFileDialog.TryFetchResultFilePath(l_resultFilePath))
    {
        auto& l_sceneManager          = SceneManager::GetInstance                        ();
        auto& l_application           = Application::GetInstance                         ();
        auto& l_assetFilePathRegistry = l_application.GetMutableREFAssetFilePathRegistry();

        if (l_sceneManager.SaveAs(l_resultFilePath, l_assetFilePathRegistry))
        {
            FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "シーンを保存しました。\nFilePath : {}", l_resultFilePath.string());
        }
    }
}

void FWK::Editor::FileMainMenuEditor::NewScene() const
{
    auto& l_sceneManager = SceneManager::GetInstance();

    l_sceneManager.RequestNewScene();
}
void FWK::Editor::FileMainMenuEditor::OpenScene()
{
    m_openSceneFileDialog.RequestOpen(CreateSceneFileDialogDesc(k_openSceneFileDialogTitle));
}
void FWK::Editor::FileMainMenuEditor::SaveScene()
{
    const auto& l_sceneManager         = SceneManager::GetInstance               ();
    const auto& l_currentSceneFilePath = l_sceneManager.GetREFCurrentSceneFilePath();

    // 新規作成したシーンはまだ保存先が無いので、「名前を付けて保存」として保存先を選んでもらう
    if (l_currentSceneFilePath.empty())
    {
        SaveSceneAs();

        return;
    }

    if (!l_sceneManager.Save()) { return; }

    FWK_ADD_LOG(Constant::k_imguiDebugSuccessColor, "シーンを保存しました。\nFilePath : {}", l_currentSceneFilePath.string());
}
void FWK::Editor::FileMainMenuEditor::SaveSceneAs()
{
    m_saveSceneFileDialog.RequestSave(CreateSceneFileDialogDesc(k_saveSceneFileDialogTitle));
}

bool FWK::Editor::FileMainMenuEditor::CanOperateScene() const
{
    const auto& l_sceneManager = SceneManager::GetInstance();

    // 再生中のシーンは▶を押した後に動いた状態なので、保存も作り直しもさせない
    // ダイアログを表示している間は、二つ目のダイアログや別の操作を受け付けない
    return !l_sceneManager.FetchVALIsPlaying()  &&
           !m_openSceneFileDialog.IsShowing() &&
           !m_saveSceneFileDialog.IsShowing();
}

FWK::Struct::FileDialogDesc FWK::Editor::FileMainMenuEditor::CreateSceneFileDialogDesc(const std::wstring_view& a_title) const
{
    const auto& l_application          = Application::GetInstance                 ();
    const auto& l_window               = l_application.GetREFWindow               ();
    const auto& l_sceneManager         = SceneManager::GetInstance                ();
    const auto& l_currentSceneFilePath = l_sceneManager.GetREFCurrentSceneFilePath();

    Struct::FileDialogDesc l_desc = {};

    // メインウィンドウを親にすることで、ダイアログを閉じるまでエディターを操作できないようにする
    l_desc.m_ownerHWND = l_window.GetREFHWND();

    // 今開いているシーンのフォルダから開く(新規シーンならAssetフォルダから開く)
    l_desc.m_initialDirectoryPath = l_currentSceneFilePath.empty() ? Constant::k_assetRootFolderPath : l_currentSceneFilePath.parent_path();

    l_desc.m_title            = a_title;
    l_desc.m_filterName       = Constant::k_jsonFileDialogFilterName;
    l_desc.m_filterPattern    = Constant::k_jsonFileDialogFilterPattern;
    l_desc.m_defaultExtension = Constant::k_jsonFileDialogDefaultExtension;

    return l_desc;
}