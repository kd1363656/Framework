#pragma once

namespace FWK::Editor
{
	class FileMainMenuEditor final : public EditorMainMenuBase
	{
	public:

		 FileMainMenuEditor()          = default;
		~FileMainMenuEditor() override = default;

		void Draw() override;

	private:

		void DrawMenuFile     () const;
		void UpdateShortCutKey() const;
		
		static constexpr std::string_view k_beginFileMenuTextString = "ファイル";
		static constexpr std::string_view k_saveTextString          = "保存";
		static constexpr std::string_view k_saveShortCutString      = "Ctrl+S";

		FWK_DEFINE_TYPE_INFO(FileMainMenuEditor, EditorMainMenuBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorMainMenuUniqueFactory, FWK::Editor::FileMainMenuEditor)