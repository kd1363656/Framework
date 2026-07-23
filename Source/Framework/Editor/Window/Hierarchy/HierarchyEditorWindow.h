#pragma once

namespace FWK::Editor
{
	class HierarchyEditorWindow final : public EditorWindowBase
	{
	public:

		 HierarchyEditorWindow()          = default;
		~HierarchyEditorWindow() override = default;

		void Draw() override;

	private:

		static constexpr std::string_view k_editorName = "Hierarchy";

		FWK_DEFINE_TYPE_INFO(HierarchyEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::HierarchyEditorWindow)