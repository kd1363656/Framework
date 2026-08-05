#include "ContentBrowserEditorWindow.h"

void FWK::Editor::ContentBrowserEditorWindow::Draw()
{
	if (!ImGui::Begin(k_editorName.data()))
	{
		ImGui::End();

		return;
	}

	ImGui::End();
}

void FWK::Editor::ContentBrowserEditorWindow::CreatePrefab(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_prefabFilePath)
{

}

std::string FWK::Editor::ContentBrowserEditorWindow::CreateVALPrefabName(const GameObject& a_gameObject) const
{
	return std::string();
}

std::filesystem::path FWK::Editor::ContentBrowserEditorWindow::CreateVALPrefabFilePath(const std::filesystem::path& a_directoryPath, const std::string& a_prefabName) const
{
	return std::filesystem::path();
}

void FWK::Editor::ContentBrowserEditorWindow::RegisterPrefabAsset(const std::string& a_prefabName, const Struct::PrefabData& a_prefabData)
{

}

void FWK::Editor::ContentBrowserEditorWindow::SetupPrefabData(const std::weak_ptr<GameObject>& a_gameObject, const std::filesystem::path& a_prefabFilePath, Struct::PrefabData& a_outPrefabData) const
{
}

void FWK::Editor::ContentBrowserEditorWindow::DrawDirectoryTree()
{
}

void FWK::Editor::ContentBrowserEditorWindow::DrawCurrentDirectoryHeader()
{
}

void FWK::Editor::ContentBrowserEditorWindow::DrawCurrentDirectoryDropTarget()
{
}

void FWK::Editor::ContentBrowserEditorWindow::ReceiveDroppedGameObject(const std::filesystem::path & a_dropDirectoryPath)
{
}

bool FWK::Editor::ContentBrowserEditorWindow::CanCreatePrefabAsset(const GameObject& a_gameObject, const std::filesystem::path& a_directoryPath) const
{
	return false;
}