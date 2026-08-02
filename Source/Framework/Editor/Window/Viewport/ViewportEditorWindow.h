#pragma once

namespace FWK::Editor
{
	class ViewportEditorWindow final : public EditorWindowBase
	{
	public:

		 ViewportEditorWindow();
		~ViewportEditorWindow() override;

		void PostDeserialize() override;

		void Draw() override;

		void SetupViewportTextureDescriptors();

	private:

		ImTextureID FetchVALViewportTextureID() const;

		void DrawViewportTexture(const ImTextureID& a_textureID, const ImVec2& a_viewportSize) const;

		static constexpr std::string_view k_editorName = "Viewport";

		static constexpr float k_minViewportSize = 1.0F;

		static constexpr float k_viewportUVMINX = 0.0F;
		static constexpr float k_viewportUVMINY = 0.0F;
		static constexpr float k_viewportUVMAXX = 1.0F;
		static constexpr float k_viewportUVMAXY = 1.0F;

		static constexpr ImTextureID k_invalidViewportTextureID = {};

		std::vector<TypeAlias::DescriptorIndex> m_imGuiSRVDescriptorIndexList = {};

		ViewportToolbar m_toolbar = {};

		FWK_DEFINE_TYPE_INFO(ViewportEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ViewportEditorWindow)