#include "WorldOutlinerEditorWindowPopupDrawer.h"

void FWK::Editor::WorldOutlinerEditorWindowPopupDrawer::BeginPopup(const std::string_view& a_openPopupLabel) const
{
    // OpenPopupは呼び出した時点のIDスタックでポップアップIDを作るため
    // BeginPopupと同じスコープ(同じラベル)で呼ぶ必要がある
    ImGui::OpenPopup(a_openPopupLabel.data());
}

void FWK::Editor::WorldOutlinerEditorWindowPopupDrawer::DrawScenePopup(const std::string_view&          a_popupLabel,
                                                                             Scene&                     a_scene, 
                                                                             WorldOutlinerEditorWindow& a_editorWindow, 
                                                                             EditorManager&             a_editorManager)
{
    if (!ImGui::BeginPopup(a_popupLabel.data())) { return; }

    // 上下キーとマウスで項目を選べるようにする
    m_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Vertical);
 
    // 親を指定しない = シーンのルートへ追加
    DrawCreateEmptyGameObjectMenu({}, 
                                  a_scene,
                                  a_editorWindow,
                                  a_editorManager);
 
    // Clipboardが空ならPasteは無効化する
    // 第3引数が有効/無効、falseならグレーアウトされて押せない
    if (m_popupNavigator.MenuItem(k_pasteMenuLabel.data(), k_pasteShortcutLabel.data(), !a_editorWindow.GetREFClipboard().IsEmpty()))
    {
        auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
        const auto& l_gameObjectOperation = a_editorWindow.GetREFGameObjectOperation();
 
        l_gameObjectOperation.PasteGameObjects(a_editorWindow.GetMutableREFClipboard(), l_gameObjectSelectionState, a_scene);
    }

    ImGui::Separator();
 
    // シーン名の変更
    if (m_popupNavigator.MenuItem(k_renameMenuLabel.data(), k_renameShortcutLabel.data()))
    {
        a_editorWindow.StartSceneRename(a_scene);
    }
 
    m_popupNavigator.End();

    ImGui::EndPopup();
}
void FWK::Editor::WorldOutlinerEditorWindowPopupDrawer::DrawGameObjectPopup(const std::weak_ptr<GameObject>& a_gameObject, 
                                                                            const std::string_view&          a_popupLabel, 
                                                                                  Scene&                     a_scene, 
                                                                                  WorldOutlinerEditorWindow& a_editorWindow, 
                                                                                  EditorManager&             a_editorManager)
{
    if (!ImGui::BeginPopup(a_popupLabel.data())) { return; }

    // 上下キーとマウスで項目を選べるようにする
    m_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Vertical);

    // 右クリックしたノードの子として空のGameObjectを追加する
    DrawCreateEmptyGameObjectMenu(a_gameObject, 
                                  a_scene,
                                  a_editorWindow, 
                                  a_editorManager);

    ImGui::Separator();

    // リネームポップアップの描画
    if (m_popupNavigator.MenuItem(k_renameMenuLabel.data(), k_renameShortcutLabel.data()))
    {
        a_editorWindow.StartGameObjectRename(a_gameObject);
    }

          auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
    const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

    // コピー
    if (m_popupNavigator.MenuItem(k_copyMenuLabel.data(), k_copyShortcutLabel.data()))
    {
        l_gameObjectOperation.CopySelectedGameObjects(l_gameObjectSelectionState, a_editorWindow.GetMutableREFClipboard());
    }
 
    // 切り取り
    if (m_popupNavigator.MenuItem(k_cutMenuLabel.data(), k_cutShortcutLabel.data()))
    {
        l_gameObjectOperation.CutSelectedGameObjects(a_editorWindow.GetMutableREFClipboard(), l_gameObjectSelectionState);
    }
 
    // 貼り付け
    // Clipboardが空なら無効化する
    // 右クリック時点で対象ノードは選択済みになるため
    // 選択中のGameObjectそれぞれの子へ貼り付けられる
    if (m_popupNavigator.MenuItem(k_pasteMenuLabel.data(), k_pasteShortcutLabel.data(), !a_editorWindow.GetREFClipboard().IsEmpty()))
    {
        l_gameObjectOperation.PasteGameObjects(a_editorWindow.GetMutableREFClipboard(), l_gameObjectSelectionState, a_scene);
    }

    // 複製
    if (m_popupNavigator.MenuItem(k_duplicateMenuLabel.data(), k_duplicateShortcutLabel.data()))
    {
        l_gameObjectOperation.DuplicateSelectedGameObjects(l_gameObjectSelectionState, a_scene);
    }
 
    // 右クリック時点で対象ノードは選択済みになるため
    // 選択中のGameObject全てを削除対象にする(複数選択にも対応)
    if (m_popupNavigator.MenuItem(k_deleteMenuLabel.data(), k_deleteShortcutLabel.data()))
    {
        l_gameObjectOperation.DestroySelectedGameObjects(l_gameObjectSelectionState);
    }

    m_popupNavigator.End();

    ImGui::EndPopup();
}
void FWK::Editor::WorldOutlinerEditorWindowPopupDrawer::DrawEmptySpacePopup(const std::string_view&          a_popupLabel, 
                                                                                  Scene&                     a_scene, 
                                                                                  WorldOutlinerEditorWindow& a_editorWindow, 
                                                                                  EditorManager&             a_editorManager)
{
    if (!ImGui::BeginPopup(a_popupLabel.data())) { return; }

    // 上下キーとマウスで項目を選べるようにする
    m_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Vertical);

    // 空スペースのポップアップは空のGameObject作成のみを持つ
    DrawCreateEmptyGameObjectMenu({},
                                  a_scene,
                                  a_editorWindow, 
                                  a_editorManager);

    // Clipboardが空ならPasteは無効化する
    if (const auto& l_clipboard = a_editorWindow.GetREFClipboard();
        m_popupNavigator.MenuItem(k_pasteMenuLabel.data(), k_pasteShortcutLabel.data(), !l_clipboard.IsEmpty()))
    {
              auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
        const auto& l_gameObjectOperation      = a_editorWindow.GetREFGameObjectOperation             ();

        l_gameObjectOperation.PasteGameObjects(a_editorWindow.GetMutableREFClipboard(), l_gameObjectSelectionState, a_scene);
    }

    m_popupNavigator.End();

    ImGui::EndPopup();
}

void FWK::Editor::WorldOutlinerEditorWindowPopupDrawer::DrawCreateEmptyGameObjectMenu(const std::weak_ptr<GameObject>& a_parent, 
                                                                                            Scene&                     a_scene,
                                                                                            WorldOutlinerEditorWindow& a_editorWindow,
                                                                                            EditorManager&             a_editorManager)
{
    if (!m_popupNavigator.MenuItem(k_createEmptyGameObjectMenuLabel.data())) { return; }
 
    const auto& l_assetCreator      = a_editorWindow.GetREFAssetCreator   ();
    const auto& l_createdGameObject = l_assetCreator.CreateEmptyGameObject(a_parent, a_scene);
 
    if (!l_createdGameObject) { return; }
 
    // 作成したGameObjectを選択状態にする
    auto& l_gameObjectSelectionState = a_editorManager.GetMutableREFGameObjectSelectionState();
 
    l_gameObjectSelectionState.SelectSingleGameObject(l_createdGameObject);
 
    auto& l_sceneSelectionState = a_editorWindow.GetMutableREFSceneSelectionState();

    l_sceneSelectionState.SetIsSceneSelected(false);

    // 生成をUndoRedoへ登録する
    // 親と並び順はコマンドが作成直後の状態から記録する
    auto& l_undoRedoSystem = a_editorManager.GetMutableREFUndoRedoSystem();

    const std::vector<std::weak_ptr<GameObject>> l_createdGameObjectList = { l_createdGameObject };

    l_undoRedoSystem.PushUndoCommand<CreateGameObjectCommand>(l_createdGameObjectList);
}