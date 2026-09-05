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

		bool Synchronize(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager);

		void Release();

	private:

		bool PrepareNotificationRead();

		bool ProcessNotificationBuffer(const DWORD& a_transferredByteSize);

		bool ProcessNotification(const FILE_NOTIFY_EXTENDED_INFORMATION& a_notificationInformation);

		bool StorePendingRenameAsDelete();

		void StoreAddChange     (const std::filesystem::path& a_filePath,    const bool                   a_isDirectory);
		void StoreDeleteChange  (const std::filesystem::path& a_filePath,    const bool                   a_isDirectory);
		void StoreFilePathChange(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const bool a_isDirectory);

		void ApplyDirectoryChangeList(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager);

		void ResetPendingRename();

		static constexpr std::size_t k_notificationBufferByteSize = 64ULL * 1024ULL;
		static constexpr std::size_t k_initialBufferOffset        = 0ULL;

		static constexpr DWORD k_noWaitMilliseconds                = 0UL;
		static constexpr DWORD k_directoryChangeNotificationFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME;
		static constexpr DWORD k_initialTransferredByteSize        = 0UL;

		static constexpr std::int64_t k_initialPendingRenameFileID = 0LL;

		std::vector<std::unique_ptr<AssetBrowserEditorWindowDirectoryChangeBase>> m_directoryChangeList;

		alignas(DWORD) std::array<std::byte, k_notificationBufferByteSize> m_notificationBufferList;

		HANDLE m_directoryHandle;
		HANDLE m_notificationEventHandle;

		OVERLAPPED m_overlapped;

		std::filesystem::path m_directoryPath;
		std::filesystem::path m_pendingRenameOldFilePath;

		std::int64_t m_pendingRenameFileID;

		bool m_isPendingRenameDirectory;
		bool m_isPendingRename;
		bool m_isNotificationReadPending;
	};
}