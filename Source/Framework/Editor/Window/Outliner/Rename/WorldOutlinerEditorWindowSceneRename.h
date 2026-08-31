#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowSceneRename final
	{
	public:

		 WorldOutlinerEditorWindowSceneRename() = default;
		~WorldOutlinerEditorWindowSceneRename() = default;

		void Update(const std::weak_ptr<Scene>& a_scene, const bool a_isSceneSelected);

		void DrawRenameInput();

		void ConfirmRename();

		void CancelRename();

		bool IsTarget(const std::weak_ptr<Scene>& a_scene) const;

		bool GetVALIsRenameActive() const { return m_isRenameActive; }

	private:

		void RequestRename(const std::weak_ptr<Scene>& a_scene);

		void ClearRenameState();

		static constexpr std::string_view k_sceneRenameInputLabel = "###SceneRenameInput";

		std::weak_ptr<Scene> m_targetScene = {};

		std::string m_renameBuffer = {};

		bool m_isRenameActive              = false;
		bool m_isRenameInputFocusRequested = false;
	};
}