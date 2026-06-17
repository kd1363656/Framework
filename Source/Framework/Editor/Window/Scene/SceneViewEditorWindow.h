#pragma once

namespace FWK::Editor
{
	class SceneViewEditorWindow final : public EditorWindowBase
	{
	public:

		 SceneViewEditorWindow()	;
		~SceneViewEditorWindow() override;

		void PostDeserialize() override;

		void Draw() override;

	private:

		ImTextureID FetchVALSceneViewTextureID();

		void DrawSceneViewTexture(const ImTextureID& a_textureID, const ImVec2& a_sceneViewSize) const;

		static constexpr std::string_view k_editorName = "Scene View";

		static constexpr float k_minSceneViewSize = 1.0F;

		static constexpr float k_sceneViewUVMINX = 0.0F;
		static constexpr float k_sceneViewUVMINY = 0.0F;
		static constexpr float k_sceneViewUVMAXX = 1.0F;
		static constexpr float k_sceneViewUVMAXY = 1.0F;

		static constexpr ImTextureID k_invalidSceneViewTextureID = {};

		TypeAlias::DescriptorIndex m_imGuiSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;

		FWK_DEFINE_TYPE_INFO(SceneViewEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::SharedFactoryEditorWindow, FWK::Editor::SceneViewEditorWindow)