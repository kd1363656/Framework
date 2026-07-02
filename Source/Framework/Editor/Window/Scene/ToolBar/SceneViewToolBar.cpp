#include "SceneViewToolBar.h"

void FWK::Editor::SceneViewToolbar::Draw()
{
	if (const auto& l_toolbarSize = ImVec2{ ImGui::GetContentRegionAvail().x, k_toolbarHeight };
		!ImGui::BeginChild(k_toobbarChildID.data(),
						   l_toolbarSize,
						   false,
						   ImGuiWindowFlags_NoScrollbar | 
						   ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::EndChild();
		return;
	}

	DrawDisplayOptionsButton();

	ImGui::EndChild();
}

void FWK::Editor::SceneViewToolbar::DrawDisplayOptionsButton() const
{
	if (ImGui::Button("表示"))
	{
		ImGui::OpenPopup(k_displayOptionsPopupID.data());
	}

	DrawDisplayOptionsPopup();
}
void FWK::Editor::SceneViewToolbar::DrawDisplayOptionsPopup() const
{
	if (!ImGui::BeginPopup(k_displayOptionsPopupID.data())) { return; }

	DrawCollisionMenuItem();

	ImGui::EndPopup();
}
void FWK::Editor::SceneViewToolbar::DrawCollisionMenuItem() const
{
	auto& l_physicsManager = Physics::PhysicsManager::GetInstance();

	const bool l_isEnabled = l_physicsManager.GetVALIsDisableDebugDraw();

	if (ImGui::MenuItem(k_collisionMenuItemLabel.data(), nullptr, l_isEnabled))
	{
		l_physicsManager.TogglePhysicsDebugDraw();
	}
}