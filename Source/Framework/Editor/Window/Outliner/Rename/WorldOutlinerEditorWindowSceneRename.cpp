#include "WorldOutlinerEditorWindowSceneRename.h"

void FWK::Editor::WorldOutlinerEditorWindowSceneRename::Update(const std::weak_ptr<Scene>& a_scene, const bool a_isSceneSelected)
{
	const auto& l_scene = a_scene.lock();

	// Rename中にSceneが切り替わった場合や、
	// SceneNodeの選択が解除された場合は、
	// 古いSceneに対するRename状態を残さない
	if (m_isRenameActive)
	{
		const auto& l_targetScene = m_targetScene.lock();

		if (!a_isSceneSelected ||
			!l_scene           ||
			!l_targetScene     ||
			l_scene != l_targetScene)
		{
			CancelRename();
		}
	}

	// 既にRename中なrあ新しいF2操作は受け取らない
	// SceneNodeが選択されていない場合はF2をSceneRenameとして扱わない
	if (m_isRenameActive ||
		!l_scene         ||
		!a_isSceneSelected)
	{
		return; 
	}
	
	// OutlinerにFocusがある場合だけF2を受け取る
	if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) ||
		!ImGui::IsKeyPressed(ImGuiKey_F2, false))
	{
		return;
	}

	RequestRename(a_scene);
}

void FWK::Editor::WorldOutlinerEditorWindowSceneRename::DrawRenameInput()
{
	if (!m_isRenameActive) { return; }

	// F2を押した直後だけInputTextへKeyboardFocusを移す
	if (m_isRenameInputFocusRequested)
	{
		ImGui::SetKeyboardFocusHere();

		m_isRenameInputFocusRequested = false;
	}

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

	const bool l_isRenameConfirmByEnter = ImGui::InputText(k_sceneRenameInputLabel.data(), 
		                                                   &m_renameBuffer,
		                                                   ImGuiInputTextFlags_EnterReturnsTrue |
	                                                       ImGuiInputTextFlags_AutoSelectAll);

	const bool l_isRenameInputHovered = ImGui::IsItemHovered();

	// Escapeでは変更を破棄する
	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		CancelRename();

		return;
	}

	// Enterでは変更を確定する
	if (l_isRenameConfirmByEnter)
	{
		ConfirmRename();

		return;
	}

	// InputTet自身のクリックは文字編集操作なので確定しない
	if (l_isRenameInputHovered) { return; }

	// InputText以外をクリックした場合は現在の名前を確定する
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
		ImGui::IsMouseClicked(ImGuiMouseButton_Right))
	{
		ConfirmRename();
	}
}

void FWK::Editor::WorldOutlinerEditorWindowSceneRename::ConfirmRename()
{
	if (!m_isRenameActive) { return; }

	const auto& l_scene = m_targetScene.lock();

	if (!l_scene)
	{
		ClearRenameState();

		return;
	}

	// Sceneは必ず表示名を持つ設計にするため
	// 空文字でのRenameは許可しない
	if (m_renameBuffer.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneNameが空のため、SceneのRenameを確定できませんでした。");

		CancelRename();

		return;
	}

	l_scene->SetSceneName(m_renameBuffer);

	ClearRenameState();
}

void FWK::Editor::WorldOutlinerEditorWindowSceneRename::CancelRename()
{
	if (!m_isRenameActive) { return; }

	ClearRenameState();
}

bool FWK::Editor::WorldOutlinerEditorWindowSceneRename::IsTarget(const std::weak_ptr<Scene>& a_scene) const
{
	if (!m_isRenameActive) { return false; }

	const auto& l_scene       = a_scene.lock      ();
	const auto& l_targetScene = m_targetScene.lock();

	if (!l_scene ||
		!l_targetScene)
	{
		return false;
	}

	return l_scene == l_targetScene;
}

void FWK::Editor::WorldOutlinerEditorWindowSceneRename::RequestRename(const std::weak_ptr<Scene>& a_scene)
{
	const auto& l_scene = a_scene.lock();

	if (!l_scene ||
		l_scene->GetREFSceneName().empty())
	{
		return;
	}

	m_targetScene  = a_scene;
	m_renameBuffer = l_scene->GetREFSceneName();

	m_isRenameActive              = true;
	m_isRenameInputFocusRequested = true;
}

void FWK::Editor::WorldOutlinerEditorWindowSceneRename::ClearRenameState()
{
	m_targetScene.reset ();
	m_renameBuffer.clear();

	m_isRenameActive              = false;
	m_isRenameInputFocusRequested = false;
}