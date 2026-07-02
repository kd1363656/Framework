#pragma once

namespace FWK::Editor
{
	class SceneViewEditorWindow final : public EditorWindowBase
	{
	public:

		 SceneViewEditorWindow();
		~SceneViewEditorWindow() override;

		void PostDeserialize() override;

		void Draw() override;

		void SetupSceneViewTextureDescriptors();

	private:

		ImTextureID FetchVALSceneViewTextureID() const;

		void DrawSceneViewTexture(const ImTextureID& a_textureID, const ImVec2& a_sceneViewSize) const;

		static constexpr std::string_view k_editorName = "Scene View";

		static constexpr float k_minSceneViewSize = 1.0F;

		static constexpr float k_sceneViewUVMINX = 0.0F;
		static constexpr float k_sceneViewUVMINY = 0.0F;
		static constexpr float k_sceneViewUVMAXX = 1.0F;
		static constexpr float k_sceneViewUVMAXY = 1.0F;

		static constexpr ImTextureID k_invalidSceneViewTextureID = {};

		std::vector<TypeAlias::DescriptorIndex> m_imGuiSRVDescriptorIndexList = {};

		SceneViewToolbar m_sceneViewToolbar = {};

		FWK_DEFINE_TYPE_INFO(SceneViewEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::SceneViewEditorWindow)