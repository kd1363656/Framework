#include "AssetBrowserEditorWindowDirectoryNotificationProcessor.h"

bool FWK::Editor::AssetBrowserEditorWindowDirectoryNotificationProcessor::ProcessNotificationBuffer(const std::filesystem::path& a_directoryPath, const DWORD& a_writtenByteSize)
{
	const auto& l_writtenByteSize                 = static_cast<std::size_t>  (a_writtenByteSize);
	const auto& l_previousDirectoryChangeListSize = m_directoryChangeList.size();
	      auto  l_bufferOffset                    = k_initialBufferOffset;
		  bool  l_requiresFolderTreeRefresh       = false;

	// Windowsが今回使用したと報告したバイト数が
	// 自身が確保している通知Buffer容量を超えることは正常ではありえない。
	// そのような状態で解析するとBuffer外Accessになる危険があるため中断する
	if (l_writtenByteSize > m_notificationBufferList.size())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知Byte数が通知Buffer容量を超えています。");

		// Old側通知とNew側通知の対応関係も信用できなくなるため
		// Pending情報を破棄する
		ResetPendingFilePathChange();

		return true;
	}

	// FILE_NOTIFY_EXTENDED_INFORMATIONは、FileNameだけが可変長になっている。
	// offsetof(Type, Member)を使うと、FILE_NOTIFY_EXTENDED_INFORMATION先頭から
	// 可変長FileNameの直前までの固定Sizeを取得できる
	const auto& l_notificationFixedByteSize = offsetof(FILE_NOTIFY_EXTENDED_INFORMATION, FileName);

	// Windowsは一回のReadDirectoryChangesExW()で、
	// 複数の通知Recordを一つのBufferへ連続して書き込む
	// そのため、Windowsが実際に書き込んだ範囲が残っている間、
	// 1Recordずつ解析する
	while (l_bufferOffset < l_writtenByteSize)
	{
		// 現在位置から、通知Bufferの有効Data末尾まで
		// あと何Byte残っているかを計算する
		// l_writtenByteSize = 500
		// l_bufferOffset    = 200
		// 500 - 200         = 残り300Byte
		const auto& l_remainingByteSize = l_writtenByteSize - l_bufferOffset;

		// FileName以外の固定部分すら残っていないなら、
		// FILE_NOTIFY_EXTENDED_INFORMATIONとして独立しない
		// 例えば固定部分が80Byte必要なのに
		// 残り20Byteしかなければ通知Datが途中で切れている
		if (l_remainingByteSize < l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知Bufferに不正なDataが含まれています。");

			// 現在のリストのサイズを過去のリストのサイズに戻す
			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		// Windowsの通知Bufferはstd::byteの生Dataなので、
		// そのままでは
		// .Action
		// .FileID
		// .FileNameLength
		// のようにMemberへAccessできない。
		// そこでFilenameを除いた固定部分だけを、
		// FILE_NOTIFY_EXTENDED_INFORMATION型のLocal変数へコピーする
		FILE_NOTIFY_EXTENDED_INFORMATION l_notificationInformation = {};

		// コピー元   : m_notificationBufferList.data() + l_bufferOffset(つまり現在読んでいる通知Recordの先頭Address)
		// コピーSize : l_notificationFixedByteSize(Filenameは可変長なのでこの時点ではコピーしない)
		// つまりOffsetはFILE_NOTIFY_EXTENDED_INFORMATIONの次の位置を先頭を示すためこのようなコピー方法を取っている
		std::memcpy(&l_notificationInformation, m_notificationBufferList.data() + l_bufferOffset, l_notificationFixedByteSize);

		const auto& l_fileNameByteSize = static_cast<std::size_t>(l_notificationInformation.FileNameLength);

		// FileNameLengthを検証する
		// 0Byte : FileNameが存在しないため不正
		// sizeof(WCHAR)で割り切れない
		// FileNameはWCHAR配列なので、
		// 例えば11Byteのような値では
		// WCHAR単位の文字列として成立しない
		if (l_notificationInformation.FileNameLength == static_cast<DWORD>(NULL) ||
			l_notificationInformation.FileNameLength % sizeof(WCHAR) != static_cast<DWORD>(NULL))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorが無効なFileNameLengthを受け取りました。");

			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		// 現在Recordに残っている領域の中へ
		// FileName全体が本当に収まっているか確認する
		// l_remainingByteSize - l_notificationFixedByteSizeで、
		// 固定部分を除いた後、FileName用として利用可能な最大Byte数
		// を算出している
		if (l_fileNameByteSize > l_remainingByteSize - l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorの通知BufferにBuffer範囲外のFileNameLengthが含まれています。");

			m_directoryChangeList.resize(l_previousDirectoryChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		// 現愛通知Recordが実際に必要とするByte数
		// 固定部分 + FileName部分で必要なByte数を算出
		const auto& l_recordByteSize= l_notificationFixedByteSize + l_fileNameByteSize;

		// ファイルのByte数からWCHARを割って何文字必要かを算出
		const auto& l_fileNameCharacterCount = l_fileNameByteSize / sizeof(WCHAR);

		// 算出した文字列数分NULLもじでstd::wstringを初期化
		std::wstring l_relativeFilePathString(l_fileNameCharacterCount, Constant::k_wNullCharacter);

		// バッファーから現在のバッファー位置と構造体固定Byte数分を足した位置かファイル名のByte数分コピーして文字列をコピーする
		std::memcpy(l_relativeFilePathString.data(), m_notificationBufferList.data() + l_bufferOffset + l_notificationFixedByteSize, l_fileNameByteSize);

		// Windowが通知するFileNameは監視Root空見た相対Path
		const auto& l_filePath = a_directoryPath / std::filesystem::path{ l_relativeFilePathString };

		// 生バッファーから1件の通知を安全に取り出したためそのファイルが
		// ADDED/REMOVED\REBAME_OLD_NAME/RENAME_NEW_NAME七日の判別を行う
		l_requiresFolderTreeRefresh = ProcessNotification(l_filePath, l_notificationInformation) ||
			                          l_requiresFolderTreeRefresh;

		// 0なら現在Recordが最後に取得できるFILE_NOTIFY_EXTENDED_INFORMATION
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

		// オフセット位置を解析した分進める
		l_bufferOffset += l_nextEntryOffset;
	}

	return l_requiresFolderTreeRefresh;
}