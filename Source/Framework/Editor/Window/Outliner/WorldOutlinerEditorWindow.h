#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindow;
}

namespace FWK::Editor
{
	class WorldOutlinerEditorWindow final : public EditorWindowBase
	{
	public:

		 WorldOutlinerEditorWindow()          = default;
		~WorldOutlinerEditorWindow() override = default;

		void Draw() override;

	private:

		static constexpr std::string_view k_editorName = "アウトライナー";
		
		FWK_DEFINE_TYPE_INFO(WorldOutlinerEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::WorldOutlinerEditorWindow)