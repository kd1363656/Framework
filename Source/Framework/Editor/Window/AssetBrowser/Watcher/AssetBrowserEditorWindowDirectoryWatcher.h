#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryWatcher final
	{
	public:

		 AssetBrowserEditorWindowDirectoryWatcher();
		~AssetBrowserEditorWindowDirectoryWatcher();

		AssetBrowserEditorWindowDirectoryWatcher(const AssetBrowserEditorWindowDirectoryWatcher&)  = delete;
		AssetBrowserEditorWindowDirectoryWatcher(      AssetBrowserEditorWindowDirectoryWatcher&&) = delete;

		AssetBrowserEditorWindowDirectoryWatcher& operator=(const AssetBrowserEditorWindowDirectoryWatcher&)  = delete;
		AssetBrowserEditorWindowDirectoryWatcher& operator=(      AssetBrowserEditorWindowDirectoryWatcher&&) = delete;

		void Prepare(const std::filesystem::path& a_directoryPath);

		bool Synchronize();

		void Release();

	private:

		static constexpr DWORD k_noWaitMilliseconds = 0UL;

		static constexpr DWORD k_directoryChangeNotificationFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;

		HANDLE m_changeNotificationHandle;
	};
}