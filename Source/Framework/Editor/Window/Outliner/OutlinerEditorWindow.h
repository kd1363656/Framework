#pragma once

namespace FWK::Editor
{
	class OutlinerEditorWindow final : public EditorWindowBase
	{
	public:

		 OutlinerEditorWindow()          = default;
		~OutlinerEditorWindow() override = default;

		void Draw() override;

	private:

		void DrawGameObjectNode(const std::shared_ptr<GameObject>& a_gameObject);

		static constexpr std::string_view k_editorName = "アウトライナー";

		FWK_DEFINE_TYPE_INFO(OutlinerEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::OutlinerEditorWindow)