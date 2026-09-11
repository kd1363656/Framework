#include "AssetBrowserEditorWindowAssetPane.h"

void FWK::Editor::AssetBrowserEditorWindowAssetPane::Draw()
{
	if (!ImGui::BeginChild(k_childLabel.data(), Constant::k_imguiRemainingSize, true))
	{
		ImGui::EndChild();

		return;
	}

	ImGui::TextUnformatted(k_paneTitleLabel.data());
	ImGui::Separator      ();

	ImGui::EndChild();
}