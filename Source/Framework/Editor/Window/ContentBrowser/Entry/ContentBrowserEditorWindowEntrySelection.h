#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindowEntrySelection final
	{
	public:

		 ContentBrowserEditorWindowEntrySelection() = default;
		~ContentBrowserEditorWindowEntrySelection() = default;

		void SynchronizeCurrentFolderEntries(const std::vector<Struct::ContentBrowserEntryData>& a_currentFolderEntryList);

		void SelectSingleEntry(const std::filesystem::path& a_entryPath, const std::vector<Struct::ContentBrowserEntryData>& a_currentFolderEntryList);
		void SelectRangeEntry (const std::filesystem::path& a_entryPath, const std::vector<Struct::ContentBrowserEntryData>& a_currentFolderEntryList);
		void SelectAllEntries ();

		void ToggleEntrySelection(const std::filesystem::path& a_entryPath);
		
		void ClearSelectedEntries();

		bool ContainsSelectedEntry     (const std::filesystem::path& a_entryPath)            const;
		bool ContainsCurrentFolderEntry(const std::filesystem::path& a_selectedEntryPathSet) const;
		
		std::size_t FetchVALSelectedEntryCount() const;

		const auto& GetREFSelectedEntryPathSet() const { return m_selectedEntryPathSet; }


	private:

		void AddSelectedEntry(const std::filesystem::path& a_entryPath);

		std::unordered_set<std::filesystem::path> m_selectedEntryPathSet = {};

		std::filesystem::path m_rangeAnchorEntryPath = {};
	};
}