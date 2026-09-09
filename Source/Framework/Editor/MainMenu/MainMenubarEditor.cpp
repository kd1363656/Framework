#include "MainMenuBarEditor.h"

void FWK::Editor::MainMenuBarEditor::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::MainMenuBarEditor::Draw() const
{
	if (!ImGui::BeginMainMenuBar()) { return; }

	for (const auto& l_editorMainMenu : m_editorMainMenuList)
	{
		if (!l_editorMainMenu) { continue; }

		l_editorMainMenu->Draw();
	}

	ImGui::EndMainMenuBar();
}

nlohmann::json FWK::Editor::MainMenuBarEditor::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::MainMenuBarEditor::AddEditorMainMenu(std::unique_ptr<EditorMainMenuBase>&& a_editorMainMenu)
{
	if (!a_editorMainMenu) 
	{
		FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "追加しようとしたメインメニューが無効なため、メインメニュー追加処理に失敗しました");
		return;
	}

	m_editorMainMenuList.emplace_back(std::move(a_editorMainMenu));
}