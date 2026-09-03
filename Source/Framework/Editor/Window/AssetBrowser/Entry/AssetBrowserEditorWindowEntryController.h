#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowEntryController final
	{
	public:

		 AssetBrowserEditorWindowEntryController() = default;
		~AssetBrowserEditorWindowEntryController() = default;

		void RefreshCurrentFolderEntryList(const std::filesystem::path& a_currentFolderPath);

		void SelectSingleEntry(const std::filesystem::path& a_entryPath);
		void SelectRangeEntry (const std::filesystem::path& a_entryPath);
		void SelectAllEntries ();

		void ToggleEntrySelection(const std::filesystem::path& a_entryPath);

		void ClearSelectedEntries();

		bool ContainsSelectedEntry(const std::filesystem::path& a_entryPath) const;

		void SetCurrentFolderEntryListDirty(const bool a_set) { m_isCurrentFolderEntryListDirty = a_set; }

		std::filesystem::path FetchVALSingleSelectedFolderPath() const;

		std::size_t FetchVALSelectedEntryCount() const;

		const auto& GetREFSelectedEntryPathSet() const { return m_selectedEntryPathSet; }

		const auto& GetREFCurrentFolderEntryDataList() const { return m_currentFolderEntryDataList; }

		bool GetVALIsCurrentFolderEntryListDirty() const { return m_isCurrentFolderEntryListDirty; }

	private:

		bool ContainsCurrentFolderEntry(const std::filesystem::path& a_entryPath) const;

		void SynchronizeCurrentFolderEntries();

		static constexpr std::size_t k_singleFolderCount = 1ULL;

		static constexpr bool k_initialCurrentFolderEntryListDirty = true;

		std::unordered_set<std::filesystem::path> m_selectedEntryPathSet = {};

		std::vector<Struct::AssetBrowserEntryData> m_currentFolderEntryDataList = {};

		std::filesystem::path m_rangeAnchorEntryPath = {};

		bool m_isCurrentFolderEntryListDirty = k_initialCurrentFolderEntryListDirty;
	};
}