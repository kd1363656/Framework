#include "MainMenubarEditor.h"

void FWK::Editor::MainMenubarEditor::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::MainMenubarEditor::Draw() const
{
	if (!ImGui::BeginMainMenuBar()) { return; }

	const auto& l_editorMainMenuDataList = m_editorMainMenuSmartPointerVectorArray.GetREFArrayElementDataList();

	for (const auto& l_editorMainMenuData : l_editorMainMenuDataList)
	{
		const auto& l_editorMainMenu = l_editorMainMenuData.m_type;

		if (!l_editorMainMenu) { continue; }

		l_editorMainMenu->Draw();
	}

	ImGui::EndMainMenuBar();
}

nlohmann::json FWK::Editor::MainMenubarEditor::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::MainMenubarEditor::AddEditorMainMenu(std::unique_ptr<EditorMainMenuBase>&& a_editorMainMenu)
{
	if (!a_editorMainMenu) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "追加しようとしたメインメニューが無効なため、メインメニュー追加処理に失敗しました");
		return;
	}

	m_editorMainMenuSmartPointerVectorArray.Add(std::move(a_editorMainMenu));
}