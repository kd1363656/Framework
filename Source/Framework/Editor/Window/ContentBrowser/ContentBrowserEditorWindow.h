#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindow final : public EditorWindowBase
	{
	public:

		 ContentBrowserEditorWindow()          = default;
		~ContentBrowserEditorWindow() override = default;

		void Draw() override;

	private:

		void DrawDirectoryTree    ();
		void DrawDirectoryTreeNode(const std::filesystem::path& a_directoryPath);
		void DrawCurrentDirectory ();
		void DrawDirectoryEntry   (const std::filesystem::path& a_entryPath, bool a_isDirectory);

		bool HasChildDirectory(const std::filesystem::path& a_directoryPath) const;

		void ApplyCurrentDirectoryPath(const std::filesystem::path& a_directoryPath);

		std::string_view FetchVALDirectoryEntryIcon(const std::filesystem::path& a_entryPaath, bool a_isDirectory) const;

		inline static const std::filesystem::path k_contentRootDirectoryPath = "Content";
		
		static constexpr std::string_view k_editorName                  = "コンテンツブラウザー";
		static constexpr std::string_view k_directoryTreeChildString    = "##ContentBrowserDirectoryTree";
		static constexpr std::string_view k_currentDirectoryChildString = "##ContentBrowserCurrentDirectory";
		static constexpr std::string_view k_directoryEntryButtonString  = "##ContentBrowserDirectoryEntry";
		static constexpr std::string_view k_directoryEntryNameEllipsis  = "...";

		static constexpr float k_directoryTreePanelWidth = 240.0F;
		static constexpr float k_filleRemainingSize      = 0.0F;

		static constexpr float k_directoryEntryWidth  = 104.0F;
		static constexpr float k_directoryEntryHeight = 92.0F;

		static constexpr float k_directoryEntryIconFontSize = 48.0F;

		static constexpr float k_directoryEntryIconTopPadding    = 8.0F;
		static constexpr float k_directoryEntryTextBottomPadding = 7.0F;
		
		static constexpr float k_directoryEntryRounding = 4.0F;
		static constexpr float k_centeringRatio         = 0.5F;

		static constexpr std::size_t k_directoryEntryNameDisplayCharacterCount = 9U;

		static constexpr std::uint32_t k_minDirectoryEntryColumnCount     = 1U;
		static constexpr std::uint32_t k_initialDirectoryEntryColumnCount = 0U;

		std::filesystem::path m_currentDirectoryPath = k_contentRootDirectoryPath;
		std::filesystem::path m_selectedEntryPath    = {};

		FWK_DEFINE_TYPE_INFO(ContentBrowserEditorWindow, EditorWindowBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::EditorWindowSharedFactory, FWK::Editor::ContentBrowserEditorWindow)