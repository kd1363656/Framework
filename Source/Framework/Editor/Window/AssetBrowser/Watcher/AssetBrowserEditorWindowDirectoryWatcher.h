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

        bool Synchronize(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager);

        void Release();

    private:

        bool PrepareNotificationRead();

        static constexpr DWORD k_directoryChangeNotificationFilter = FILE_NOTIFY_CHANGE_FILE_NAME |
                                                                     FILE_NOTIFY_CHANGE_DIR_NAME;

        static constexpr DWORD k_noWaitMilliseconds     = 0UL;
        static constexpr DWORD k_initialWrittenByteSize = 0UL;

        HANDLE m_directoryHandle;
        HANDLE m_notificationEventHandle;

        AssetBrowserEditorWindowDirectoryNotificationProcessor m_notificationProcessor;

        OVERLAPPED m_overlapped;

        std::filesystem::path m_directoryPath;

        bool m_isNotificationReadPending;
    };
}