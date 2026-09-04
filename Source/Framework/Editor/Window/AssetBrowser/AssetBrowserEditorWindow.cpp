
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

	// 左ペイン、フォルダのみを表示する
	// 左クリックで右ペインの開いているフォルダを変更したりできる
	DrawFolderPane ();
	ImGui::SameLine();

	// 左ペインと同じ高さで右ペインを描画する
	// 右ペインではファイルを見ることなどができる
	DrawAssetPane();
	ImGui::End   ();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindow::Serialize()
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindow::DrawFolderPane() const
{
	const ImVec2 l_folderPaneSize = { k_folderPaneWidth, k_fileRemainingArea };

	// ImGui::BeginChild()は
	// 現在のWindowの内部にもう一つの描画領域を作成するAPI
	if (!ImGui::BeginChild(k_folderPaneChildLabel.data(), l_folderPaneSize, true))
	{
		// BeginChild()もBegin()と同様
		// 呼びだした場合は必ずEndChild()と組み合わせる
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted("フォルダ");
	ImGui::EndChild();
}
void FWK::Editor::AssetBrowserEditorWindow::DrawAssetPane() const
{
	// X = 0.0F
	// Y = 0.0F
	// とすることで、左Paneを配置した後に残っている横幅と高さを全て右Paneが使用する
	const ImVec2 l_assetPaneSize = { k_fileRemainingArea, k_fileRemainingArea };

	if (!ImGui::BeginChild(k_assetPaneChildLabel.data(), l_assetPaneSize, true))
	{
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted("アセット");
	ImGui::EndChild();
}