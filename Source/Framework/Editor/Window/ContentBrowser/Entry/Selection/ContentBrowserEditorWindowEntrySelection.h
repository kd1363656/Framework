#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindowEntrySelection final
	{
	public:

		 ContentBrowserEditorWindowEntrySelection() = default;
		~ContentBrowserEditorWindowEntrySelection() = default;

		void SynchronizeCurrentFolderEntries(const std::vector<std::filesystem::path>& a_currentFolderEntryPathList);

		void SelectSingleEntry(const std::filesystem::path& a_entryPath);
		void SelectRangeEntry (const std::filesystem::path& a_entryPath);
		void SelectAllEntries ();

		void ToggleEntrySelection(const std::filesystem::path& a_entryPath);
		
		void ClearSelectedEntries();


		bool ContainsSelectedEntry(const std::filesystem::path& a_entryPath) const;
		
		std::size_t FetchVALSelectedEntryCount() const;

		const auto& GetREFSelectedEntryPathSet() const { return m_selectedEntryPathSet; }

	private:

		bool ContainsCurrentFolderEntry(const std::filesystem::path& a_entryPath) const;

		void AddSelectedEntry(const std::filesystem::path& a_entryPath);

		std::unordered_set<std::filesystem::path> m_selectedEntryPathSet = {};

		std::vector<std::filesystem::path> m_currentFolderEntryPathList = {};

		std::filesystem::path m_rangeAnchorEntryPath = {};
	};
}