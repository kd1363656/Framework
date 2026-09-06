#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowDirectoryNotificationProcessor final
	{
	private:

		enum class PendingFilePathChangeType
		{
			Invalid,

			Removed,

			RenameOldName,
		};

		struct PendingFilePathChangeData final
		{
			static constexpr std::int64_t k_initialCreationTime = 0LL;

			PendingFilePathChangeType m_type = PendingFilePathChangeType::Invalid;

			std::filesystem::path m_oldFilePath = {};

			std::chrono::steady_clock::time_point m_registeredTime = {};

			std::int64_t m_creationTime = k_initialCreationTime;

			bool m_isDirectory = false;
		};

	public:

		 AssetBrowserEditorWindowDirectoryNotificationProcessor() = default;
		~AssetBrowserEditorWindowDirectoryNotificationProcessor() = default;

		AssetBrowserEditorWindowDirectoryNotificationProcessor(const AssetBrowserEditorWindowDirectoryNotificationProcessor&)  = delete;
		AssetBrowserEditorWindowDirectoryNotificationProcessor(      AssetBrowserEditorWindowDirectoryNotificationProcessor&&) = delete;

		AssetBrowserEditorWindowDirectoryNotificationProcessor& operator=(const AssetBrowserEditorWindowDirectoryNotificationProcessor&) = delete;
		AssetBrowserEditorWindowDirectoryNotificationProcessor& operator=(AssetBrowserEditorWindowDirectoryNotificationProcessor&&)      = delete;

		bool ProcessNotificationBuffer(const std::filesystem::path& a_directoryPath, const DWORD& a_writtenByteSize);

		bool ProcessExpiredPendingFilePathChange();

		void ApplyDirectoryChangeList(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager);

		void ResetPendingFilePathChange();

		void Release();

		std::span<std::byte> GetVALNotificationBufferList() { return std::span<std::byte>{ m_notificationBufferList }; }

	private:

		bool ProcessNotification(const std::filesystem::path& a_filePath, const FILE_NOTIFY_EXTENDED_INFORMATION& a_notificationInformation);

		void StoreAddChange     (const std::filesystem::path& a_filePath,    const bool                   a_isDirectory);
		void StoreDeleteChange  (const std::filesystem::path& a_filePath,    const bool                   a_isDirectory);
		void StoreFilePathChange(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const bool a_isDirectory);
			
		static constexpr std::chrono::milliseconds k_pendingFilePathChangeGracePeriod = std::chrono::milliseconds{ 250LL };

		static constexpr std::size_t k_notificationBufferByteSize = 64ULL * 1024ULL;
		static constexpr std::size_t k_initialBufferOffset        = 0ULL;

		std::unordered_map<std::int64_t, PendingFilePathChangeData> m_pendingFilePathChangeDataMap = {};

		std::vector<std::unique_ptr<AssetBrowserEditorWindowDirectoryChangeBase>> m_directoryChangeList = {};

		alignas(DWORD) std::array<std::byte, k_notificationBufferByteSize> m_notificationBufferList = {};
	};
}