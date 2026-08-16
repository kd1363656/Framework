#pragma once

namespace FWK::Editor
{
	class WorldOutlinerEditorWindowGameObjectRename final
	{
	public:

		 WorldOutlinerEditorWindowGameObjectRename() = default;
		~WorldOutlinerEditorWindowGameObjectRename() = default;

		void Update(const WorldOutlinerEditorWindowGameObjectSelection& a_gameObjectSelection);

		void DrawRenameInput();

		void ConfirmRename();

		void CancelRename();

		bool IsTarget(const std::weak_ptr<GameObject>& a_gameObject) const;

		bool GetVALIsRenameActive() const { return m_isRenameActive; }

	private:

		void RequestRename(const std::weak_ptr<GameObject>& a_gameObject, const WorldOutlinerEditorWindowGameObjectSelection& a_gameObjectSelection);

		void ClearRenameState();

		static constexpr std::string_view k_gameObjectRenameInputLabel = "##GameObjectRenameInput";

		static constexpr std::size_t k_singleSelectionCount = 1ULL;

		std::string m_renameBuffer = {};

		bool m_isRenameActive              = false;
		bool m_isRenameInputFocusRequested = false;
	};
}