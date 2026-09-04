
#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	ImGui::End();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindow::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}