#include "WorldOutlinerEditorWindowGameObjectRename.h"

void FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::Update(const WorldOutlinerEditorWindowGameObjectSelection& a_gameObjectSelection)
{
	const auto& l_editorManager = EditorManager::GetInstance();

	// Rename中のGameObjectの状態確認
	if (m_isRenameActive)
	{
		const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

		if (!l_selectedGameObject                                                                             ||
			l_selectedGameObject->GetVALIsDestroyed()                                                         ||
			l_selectedGameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabSceneInstanceNUM ||
			a_gameObjectSelection.FetchVALSelectedGameObjectCount() != k_singleSelectionCount)
		{
			CancelRename();
		}
	}

	// 既にRename中なら
	// 新しいF2Renameを開始しない
	if (m_isRenameActive) { return; }

	// OutlinerにFocusがある場合だけF2を受け取る
	if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
		!ImGui::IsKeyPressed(ImGuiKey_F2, false))
	{
		return; 
	}
	
	RequestRename(l_editorManager.GetREFSelectedGameObject(), a_gameObjectSelection);
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::DrawRenameInput()
{
	if (!m_isRenameActive) { return; }

	// F2を押した直後の1フレームだけ
	// InputTextへKeyboardFocusを移動する
	if (m_isRenameInputFocusRequested)
	{
		ImGui::SetKeyboardFocusHere();

		m_isRenameInputFocusRequested = false;
	}

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

	const bool l_isRenameConfirmedByEnter = ImGui::InputText(k_gameObjectRenameInputLabel.data(), 
		                                                     &m_renameBuffer,
		                                                     ImGuiInputTextFlags_EnterReturnsTrue |
	                                                         ImGuiInputTextFlags_AutoSelectAll);

	const bool l_isRenameInputHovered = ImGui::IsItemHovered();

	// Escapeなら変更を破棄する
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		CancelRename();

		return;
	}

	if (l_isRenameConfirmedByEnter) 
	{
		ConfirmRename();

		return;
	}

	// InputText事態をクリックした場合は
	// カーソル移動などの編集捜査なので確定しない
	if (l_isRenameInputHovered) { return; }

	// InputText以外を左クリック、または右クリックした場合は
	// 現在の名前を確定する
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
		ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ConfirmRename();
	}
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::ConfirmRename()
{
	if (!m_isRenameActive) { return; }

	const auto& l_editorManager      = EditorManager::GetInstance              ();
	const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

	if (!l_selectedGameObject                     ||
		l_selectedGameObject->GetVALIsDestroyed() ||
		l_selectedGameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabSceneInstanceNUM)
	{
		ClearRenameState();

		return;
	}

	// 非PrefabGameObjectではPrefasbNameを
	// Outliner上のGameObject名とし使用しているため
	// 編集Bufferの内容をそのまま設定する
	l_selectedGameObject->SetSceneInstanceName(m_renameBuffer);

	ClearRenameState();
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::CancelRename()
{
	if (!m_isRenameActive) { return; }

	ClearRenameState();
}

bool FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::IsTarget(const std::weak_ptr<GameObject>& a_gameObject) const
{
	if (!m_isRenameActive) { return false; }

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return false;
	}

    const auto& l_editorManager      = EditorManager::GetInstance              ();
	const auto& l_selectedGameObject = l_editorManager.GetREFSelectedGameObject().lock();

	if (!l_selectedGameObject ||
		l_selectedGameObject->GetVALIsDestroyed())
	{
		return false;
	}

	return l_gameObject == l_selectedGameObject;
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::RequestRename(const std::weak_ptr<GameObject>& a_gameObject, const WorldOutlinerEditorWindowGameObjectSelection& a_gameObjectSelection)
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject ||
		l_gameObject->GetVALIsDestroyed())
	{
		return;
	}

	// PrefabSceneInstanceの名前は
	// PrefabName + InstanceNUMから決定されているため
	// Outlinerから直接リネームしない
	if (l_gameObject->GetVALPrefabSceneInstanceNUM() != Constant::k_invalidPrefabSceneInstanceNUM) 
	{
		return; 
	}

	// F2リネームは単一選択時のみ行う
	if (a_gameObjectSelection.FetchVALSelectedGameObjectCount() != k_singleSelectionCount) { return; }

	// 現在Outlinerへ表示されている名前を
	// 編集用Bufferへコピーする
	m_renameBuffer = l_gameObject->FetchVALGameObjectName();

	m_isRenameActive              = true;
	m_isRenameInputFocusRequested = true;
}

void FWK::Editor::WorldOutlinerEditorWindowGameObjectRename::ClearRenameState()
{
	m_renameBuffer.clear();

	m_isRenameActive              = false;
	m_isRenameInputFocusRequested = false;
}