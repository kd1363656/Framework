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
		
		FWK_DEFINE_TYPE_INFO(FileMainMenuEditor, EditorMainMenuBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorMainMenuUniqueFactory, FWK::Editor::FileMainMenuEditor)