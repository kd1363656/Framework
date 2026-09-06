#include "AssetBrowserEditorWindowDirectoryNotificationProcessor.h"

bool FWK::Editor::AssetBrowserEditorWindowDirectoryNotificationProcessor::ProcessNotificationBuffer(const std::filesystem::path& a_directoryPath, const DWORD& a_transferredByteSize)
{
	const auto& l_transferredByteSize             = static_cast<std::size_t>  (a_transferredByteSize);
	const auto& l_previousDirectoryChangeListSize = m_directoryChangeList.size();
	      auto  l_bufferOffset                    = k_initialBufferOffset;
		  bool  l_requiresFolderTreeRefresh       = false;

	if (l_transferredByteSize > m_notificationBufferList.size())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知Byte数が通知Buffer容量を超えています。");

		ResetPendingFilePathChange();

		return true;
	}

	const auto& l_notificationFixedByteSize = offsetof(FILE_NOTIFY_EXTENDED_INFORMATION, FileName);

	while (l_bufferOffset < l_transferredByteSize)
	{
		const auto& l_remainingByteSize = l_transferredByteSize - l_bufferOffset;

		//
		if (l_remainingByteSize < l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知Bufferに不正なDataが含まれています。");

			// 現在のリストのサイズを過去のリストのサイズに戻す
			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		_FILE_NOTIFY_EXTENDED_INFORMATION l_notificationInformation = {};

		std::memcpy(&l_notificationInformation, m_notificationBufferList.data() + l_bufferOffset, l_notificationFixedByteSize);

		const auto l_fileNameByteSize = static_cast<std::size_t>(l_notificationInformation.FileNameLength);

		if (l_notificationInformation.FileNameLength == static_cast<DWORD>(NULL) ||
			l_notificationInformation.FileNameLength % sizeof(WCHAR) != static_cast<DWORD>(NULL))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorが無効なFileNameLengthを受け取りました。");

			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		if (;l_fileNameByteSize > l_remainingByteSize - l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知BufferにBuffer範囲外のFileNameLengthが含まれています。");

			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		const auto& l_recordByteSize         = l_notificationFixedByteSize + l_fileNameByteSize;
		const auto& l_fileNameCharacterCount = l_fileNameByteSize          / sizeof(WCHAR);

		std::wstring l_relativeFilePathString(l_fileNameCharacterCount, Constant::k_wNullCharacter);

		std::memcpy(l_relativeFilePathString.data(), m_notificationBufferList.data() + l_bufferOffset + l_notificationFixedByteSize, l_fileNameByteSize);

		const auto& l_filePath = a_directoryPath / std::filesystem::path{ l_relativeFilePathString };

		l_requiresFolderTreeRefresh = ProcessNotification(l_filePath, l_notificationInformation) ||
			                          l_requiresFolderTreeRefresh;

		// 0なら現在Recordが最後
		if (l_notificationInformation.NextEntryOffset == static_cast<DWORD>(NULL)) { return l_requiresFolderTreeRefresh; }

		const auto& l_nextEntryOffset = static_cast<std::size_t>(l_notificationInformation.NextEntryOffset);

		if (l_nextEntryOffset < l_recordByteSize ||
			l_nextEntryOffset >= l_remainingByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知Bufferに無効なNextEntryOffsetが含まれています。");

			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		if (l_nextEntryOffset % sizeof(DWORD) != static_cast<std::size_t>(NULL))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知BufferにDWORD境界へAlignmentされていないNextEntryOffsetが含まれています。");

			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		l_bufferOffset += l_nextEntryOffset;
	}

	return l_requiresFolderTreeRefresh;
}