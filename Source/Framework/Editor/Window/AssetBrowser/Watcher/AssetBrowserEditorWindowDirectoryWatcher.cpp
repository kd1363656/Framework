#include "AssetBrowserEditorWindowDirectoryWatcher.h"

FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::AssetBrowserEditorWindowDirectoryWatcher() : 
	m_pendingFilePathChangeDataMap(),

	m_directoryChangeList(),

	m_notificationBufferList(),

	m_directoryHandle        (INVALID_HANDLE_VALUE),
	m_notificationEventHandle(nullptr),

	m_overlapped(),

	m_directoryPath(),
	
	m_isNotificationReadPending(false)
{}
FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::~AssetBrowserEditorWindowDirectoryWatcher()
{
	Release();
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Prepare(const std::filesystem::path& a_directoryPath)
{
	// Prepare()が複数呼ばれても
	// 古いDirectoryHandleや非同期I/Oが残らないように
	// 最初に現在の監視上体をすべて解放する
	Release();

	std::error_code l_errorCode = {};

	// 監視対象が実際にDirectoryであることを確認する
	if (!std::filesystem::is_directory(a_directoryPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの監視対象Folderが存在しません。\nFolderPath : {}", a_directoryPath.string());

		return;
	}

	l_errorCode.clear();

	// CreateFileW()へ渡すため、
	// 監視対象のDirectoryをAbsolutePathへ変換する
	const std::filesystem::path l_absoluteDirectoryPath = std::filesystem::absolute(a_directoryPath, l_errorCode);

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherのAbsolutePathを取得できませんでした。\nFolderPath : {}", a_directoryPath.string());

		return;
	}

	// Windowsから帰ってくるFileNameは監視Directory基準の相対Path
	// AssetBrowser内部ではAsset/...のPathを使いたいので
	// createFileW()用AbsolutePathとは別に、
	// 呼び出し側から渡されたFramework用Pathを保持する
	m_directoryPath = a_directoryPath;

	// CreateFileW()はFileだけでなくDirectoryHANDLEも取得できる
	// FILE_LIST_DIRECTORY          : Directory内容の変更通知を受け取るためのAccess件
	// FILE_SHARE_READ/WRITE/DELETE : Watcherが監視していてもEditorやExplorer側が、Fileの読み書き・Rename・Deleteを行えるように共有する
	// FILE_FLAG_BACKUP_SEMANTICS   : CreateFileW()でDirectoryを開くために必要 
	// FILE_FLAG_OVERLAPPED         : ReadDirectoryChangeExW()を非同期で使用するために必要
	m_directoryHandle = CreateFileW(l_absoluteDirectoryPath.c_str(),
		                            FILE_LIST_DIRECTORY,
		                            FILE_SHARE_READ  |
		                            FILE_SHARE_WRITE |
		                            FILE_SHARE_DELETE,
		                            nullptr,
		                            OPEN_EXISTING,
		                            FILE_FLAG_BACKUP_SEMANTICS |
		                            FILE_FLAG_OVERLAPPED,
		                            nullptr);

	// ディレクトリハンドルが無効な場合return
	if (m_directoryHandle == INVALID_HANDLE_VALUE)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの監視Directoryを開けませんでした。\nFolderPath : {}\nWin32ErrorCode : {}", a_directoryPath.string(), GetLastError());

		Release();

		return;
	}

	// ReadDirectoryChangeExW()の非同期処理が完了すると、
	// OVERLAPPED::hEventに設定したEventがSignaledになる
	m_notificationEventHandle = CreateEventW(nullptr,
		                                     TRUE,
		                                     FALSE,
		                                     nullptr);

	if (!m_notificationEventHandle)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知Eventを作成できませんでした。\nWin32ErrorCode : {}", GetLastError());

		Release();

		return;
	}

	if (PrepareNotificationRead()) { return; }

	Release();
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Synchronize(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
	if (m_directoryHandle == INVALID_HANDLE_VALUE ||
		!m_notificationEventHandle)
	{
		return false;
	}

	// 以前のFrameでPendingになったREMOVED / OLD_NAMEの期限確認
	// File削除の場合、REMOVED -> New側追加なし -> 250ms経過 -> Delete確定となる
	bool l_requiresFolderTreeRefresh = ProcessExpiredPendingFilePathChange();

	// Eventの状態だけ確認する
	// Timeout = 0msなのでMainThreadを停止させない
	const auto l_waitResult = WaitForSingleObject(m_notificationEventHandle, k_noWaitMilliseconds);

	// まだ通知なしの場合return
	if (l_waitResult == WAIT_TIMEOUT) 
	{
		// Windows通知がなくても、
		// 前Frameで削除に失敗したAddChange等の
		// Retry処理は毎Frame実行する
		ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

		return l_requiresFolderTreeRefresh; 
	}

	if (l_waitResult == WAIT_FAILED)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知確認に失敗しました。\nWin32ErrorCode : {}", GetLastError());

		Release();

		// Watcher状態が壊れたため、
		// 念のためFolderTreeは再構築させる
		return true;
	}

	if (l_waitResult != WAIT_OBJECT_0) 
	{
		ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

		return l_requiresFolderTreeRefresh; 
	}

	DWORD l_transferredByteSize = k_initialTransferredByteSize;

	// ReadDirectoryChangesExW()は非同期で実行しているため
	// 実際にBufferへ何Byte書き込まれたかは
	// GetOverlappedResult()から取得する
	if (!GetOverlappedResult(m_directoryHandle,
		                     &m_overlapped,
	                         &l_transferredByteSize,
		                     FALSE))
	{
		const DWORD l_errorCode = GetLastError();

		// Event確認と完了状態のタイミングがわずかにずれた場合は
		// 次Frameで再確認すればいい
		if (l_errorCode == ERROR_IO_INCOMPLETE) 
		{
			ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

			return l_requiresFolderTreeRefresh; 
		}

		// ERROR_IO_INCOMPLETE以外なら、
		// 子の非同期read事態は完了扱いになる
		m_isNotificationReadPending = false;
		
		// Windowsが変更をすべて記録できなかった場合
		// この場合、個別の変更通知は信用できない
		if (l_errorCode == ERROR_NOTIFY_ENUM_DIR)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherが全ての変更通知を記録できませんでした。\nAssetDirectoryの再同期が必要です。");

			ResetPendingFilePathChange();
			
			if (!PrepareNotificationRead()) 
			{
				Release(); 
			}

			ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

			// 詳細なFolder変更も失われた可能性があるので
			// FolderTreeは再構築させる
			return true;	
		}

		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの非同期変更通知取得に失敗しました。\nWin32ErrorCode : {}", l_errorCode);

		Release();

		return true;
	}

	// GetOverlappedResult()で今回のRead結果を回収できた
	m_isNotificationReadPending = false;

	// 0Byteなら通知BufferOverflow等によって
	// 詳細な変更情報が失われている
	if (l_transferredByteSize == k_initialTransferredByteSize)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知Bufferから詳細情報を取得できませんでした。\n大量のFile変更によってBufferがOverflowした可能性があります。");

		ResetPendingFilePathChange();
		
		if (!PrepareNotificationRead())
		{
			Release();
		}

		ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

		return true;
	}

	// Windows通知Bufferを解析し、
	// AddChange / DeleteChange / FilePathChangeへ変換する
	l_requiresFolderTreeRefresh = ProcessNotificationBuffer(l_transferredByteSize) ||
		                         l_requiresFolderTreeRefresh;

	// Buffer解析中に時間が経過した可能性もあるため、
	// もう一度Pending期限を確認する
	l_requiresFolderTreeRefresh = ProcessExpiredPendingFilePathChange() ||
		                          l_requiresFolderTreeRefresh;

	// AddChange::Apply()が外部通知Jsonをremove()した場合、
	// そのremove()事態も新しいDirectory変更になる
	// Applyより先に次回readを登録することで
	// Change::Apply(9によって発生したFileSystem変更も取りこぼさない
	const bool l_isNotificationReadPrepared = PrepareNotificationRead();

	// Buffer内の情報はすでに各ChangeへPath等とコピー済みなので、
	// 同じBufferをWindowsへ再度渡しても問題ない
	ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

	if (!l_isNotificationReadPrepared) 
	{
		Release(); 
		
		return true;
	}

	return l_requiresFolderTreeRefresh;
}


void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Release()
{
	if (m_directoryHandle != INVALID_HANDLE_VALUE)
	{
		if (m_isNotificationReadPending)
		{
			// CancelIoEx()は
			// 指定HANDLE + OVERLAPPEDに対応する
			// 非同期I/OへCancel要求を出す
			// 注意 : CancelIoEx()が戻った瞬間にI/Oが完全終了したとは限らない
			// そのため後でGetOverlappedResult(TRUE)して
			// 完了を回収する
			if (!CancelIoEx(m_directoryHandle, &m_overlapped))
			{
				const auto l_errorCode = GetLastError();

				// ERROR_NOT_FOUND : Cancel対象I/Oが既に完了していた場合など
				// 以上として扱う必要はない
				if (l_errorCode != ERROR_NOT_FOUND)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの非同期I/をCancel出来ませんでした。\nWin32ErrorCode : {}", l_errorCode);
				}
			}

			// CancelIoEx()後、
			// I/OSubsystemがCancelを正式に完了するまで待つ
			// TRUE : I/Oが完了するまでここでは待機する
			// Release時のみなので毎Frame処理には影響しない
			if (auto l_transferredByteSize = k_initialTransferredByteSize;
				!GetOverlappedResult(m_directoryHandle,
				                     &m_overlapped,
				                     &l_transferredByteSize,
				                     TRUE))
			{
				const auto l_errorCode = GetLastError();

				// 正常にCancelされた非同期I/Oは
				// ERROR_OPERATION_ABORTEDで完了する
				if (l_errorCode != ERROR_OPERATION_ABORTED)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの非同期I/O終了確認に失敗しました。\nWin32ErrorCode : {}", l_errorCode);
				}
			}

			m_isNotificationReadPending = false;
		}

		if (!CloseHandle(m_directoryHandle))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherのDirectoryHandleを解放できませんでした。\nWin32ErrorCode : {}", GetLastError());
		}

		m_directoryHandle = INVALID_HANDLE_VALUE;
	}

	if (m_notificationEventHandle)
	{
		if (!CloseHandle(m_notificationEventHandle))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherのNotificationEventHandleを解放できませんでした。\nWin32ErrorCode : {}", GetLastError());
		}

		m_notificationEventHandle = nullptr;
	}

	m_overlapped = {};

	m_directoryChangeList.clear();

	m_notificationBufferList.fill(std::byte{});

	m_directoryPath.clear ();

	ResetPendingFilePathChange();
	
	m_isNotificationReadPending = false;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::PrepareNotificationRead()
{
	if (m_directoryHandle == INVALID_HANDLE_VALUE ||
		!m_notificationEventHandle)
	{
		return false;
	}

	// 同じOVERLAPPEDとBufferを使用した非同期Readを
	// 2つ同時に発行してはいけない
	if (m_isNotificationReadPending)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで既に非同期Directory監視が実行中です。");

		return false;
	}

	// CreateEventW()でManualResetEventを作成したため
	// 次の非同期Read開始前にNonSignaledへ戻す
	if (!ResetEvent(m_notificationEventHandle))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherのNotificationEventをResetできませんでした。\nWin32ErrorCode : {}", GetLastError());

		return false;
	}

	// 前回I/のInternal情報等を残さない
	m_overlapped = {};

	// この非同期I/Oが完了した際、
	// WindowsがSignaledにするEventを指定する
	m_overlapped.hEvent = m_notificationEventHandle;

	// Read\DirectoryChangeExW(CreateFileで取得したDirectoryHANDLE、
	//                         Windowsが変更情報を書き込むBuffer、
	//                         BufferのByteSize、
	//                         Trueなら監視DirectoryだけではなくSubdirectory以下もすべて再帰監視する、
	//                         何を監視するか、
	//                         非同期利用でここからByte数を取得しない、
	//                         非同期I/O用OVERLAPPED、
	//                         nullptrでCompletionRoutineは使用しない、
	//                         FILE_NOTIFY_EXTENDED_INFORMATION形式で詳細通知を取得する);
	// ReadDirectoryChangesExW()は、
	// 非同期利用の場合も監視要求のQueue登録に成功すると
	// 非0を返す
	if (!ReadDirectoryChangesExW(m_directoryHandle,
		                         m_notificationBufferList.data(),
		                         static_cast<DWORD>(m_notificationBufferList.size()),
		                         TRUE,
		                         k_directoryChangeNotificationFilter,
		                         nullptr,
		                         &m_overlapped,
		                         nullptr,
		                         ReadDirectoryNotifyExtendedInformation))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知登録に失敗しました。\nWin32ErrorCode : {}", GetLastError());

		return false;
	}

	m_isNotificationReadPending = true;

	return true;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessNotificationBuffer(const DWORD& a_transferredByteSize)
{
	const auto& l_transferredByteSize       = static_cast<std::size_t>(a_transferredByteSize);
	const auto& l_initialChangeListSize     = m_directoryChangeList.size();
	      auto  l_bufferOffset              = k_initialBufferOffset;
	      bool  l_requiresFolderTreeRefresh = false;

	// Windowsから返されたByte数が
	// 自分のBuffer容量を超えている場合は異常
	if (l_transferredByteSize > m_notificationBufferList.size())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知Byte数が通知Buffer容量を超えています。");

		ResetPendingFilePathChange();

		return true;
	}

	// FILE_NOTIFY_EXTENDED_INFORMATIONは
	// 最後のFileNameが可変長配列になっている。
	// offsetof()を使用すると、
	// 構造体先頭からFileName直前までのByte数を取得できる
	const auto& l_notificationFixedByteSize = offsetof(FILE_NOTIFY_EXTENDED_INFORMATION, FileName);

	while (l_bufferOffset < l_transferredByteSize)
	{
		const auto& l_remainingByteSize = l_transferredByteSize - l_bufferOffset;

		// 固定部分すらBuffer内に存在しなければ
		// Windowから受け取ったDataを安全に解析できない
		if (l_remainingByteSize < l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知Bufferに不正なDataが含まれています。");

			// 今回Bufferから作ったChangeだけは気
			m_directoryChangeList.resize(l_initialChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		// WindowBufferは生Byte列
		// Buffer上のAddressを直接StructPointerとして扱うのではなく
		// 固定部分をLocal変数へコピーして読む
		FILE_NOTIFY_EXTENDED_INFORMATION l_notificationInformation = {};

		std::memcpy(&l_notificationInformation, m_notificationBufferList.data() + l_bufferOffset, l_notificationFixedByteSize);

		const auto l_fileNameByteSize = static_cast<std::size_t>(l_notificationInformation.FileNameLength);

		// FileNameLengthはByte数
		// fileNameはWCHAR配列なので、
		// sizeof(WCHAR)で割り切れなければ不正
		if (l_notificationInformation.FileNameLength == static_cast<DWORD>(NULL) ||
			l_notificationInformation.FileNameLength % sizeof(WCHAR) != static_cast<DWORD>(NULL))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcher空無効なFileNameLengthを受け取りました。");

			m_directoryChangeList.resize(l_initialChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		// FileNameがBuffer末尾を超えないか確認する
		if (l_fileNameByteSize > l_remainingByteSize - l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知BufferにBuffer範囲外のFileNameLengthが含まれています。");

			m_directoryChangeList.resize(l_initialChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		const auto& l_recordByteSize         = l_notificationFixedByteSize + l_fileNameByteSize;
		const auto& l_fileNameCharacterCount = l_fileNameByteSize          / sizeof(WCHAR);

		// WindowsのFileNameはNULL終端されていない
		// fileNamelengthから正確な文字数を確保する
		std::wstring l_relativeFilePathString(l_fileNameCharacterCount, Constant::k_wNullCharacter);

		std::memcpy(l_relativeFilePathString.data(), m_notificationBufferList.data() + l_bufferOffset + l_notificationFixedByteSize, l_fileNameByteSize);

		const auto& l_filePath = m_directoryPath / std::filesystem::path{ l_relativeFilePathString };

		l_requiresFolderTreeRefresh = ProcessNotification(l_notificationInformation, l_filePath) ||
			                          l_requiresFolderTreeRefresh;

		// NextEntryOffset == NULLは
		// 現在Recordが最後という意味
		if (l_notificationInformation.NextEntryOffset == static_cast<DWORD>(NULL)) { return l_requiresFolderTreeRefresh; }

		const auto& l_nextEntryOffset = static_cast<std::size_t>(l_notificationInformation.NextEntryOffset);

		// 次Record位置が現在Record途中、Buffer末尾、Buffer外なら以上
		if (l_nextEntryOffset <  l_recordByteSize ||
			l_nextEntryOffset >= l_remainingByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知Bufferに無効なNextEntryOffsetが含まれています。");

			m_directoryChangeList.resize(l_initialChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		// windows通知RecordはDWORD境界で並ぶため、
		// 昭会にAlignmentがおかしいOffsetも不正として扱う
		if (l_nextEntryOffset % sizeof(DWORD) != static_cast<std::size_t>(NULL))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知BufferにDWORD境界へAlignmentされていない、NextEntryOffsetが含まれています。");

			m_directoryChangeList.resize(l_initialChangeListSize);

			ResetPendingFilePathChange();

			return true;
		}

		l_bufferOffset += l_nextEntryOffset;
	}

	return l_requiresFolderTreeRefresh;
}
bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessNotification(const FILE_NOTIFY_EXTENDED_INFORMATION& a_notificationInformation, const std::filesystem::path& a_filePath)
{
	if (a_filePath.empty()) { return false; }

	const bool l_isDirectory = (a_notificationInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != static_cast<DWORD>(NULL);

	// FILE_NOTIFY_EXTENDED_INFORMATIONには
	// FileSystem上のFile識別子が存在する
	// 同じFiledIDなら
	// Rename/Move前後の同一Fileとして扱える
	const auto l_fileID       = static_cast<std::int64_t>(a_notificationInformation.FileId.QuadPart);
	const auto l_creationTime = static_cast<std::int64_t>(a_notificationInformation.CreationTime.QuadPart);

	switch (a_notificationInformation.Action)
	{
		case FILE_ACTION_ADDED:
		case FILE_ACTION_RENAMED_NEW_NAME:
		{
			// New側通知
			// 同FileIdのOld側Pendingがあるか探す
			if (const auto& l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.find(l_fileID);
				l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
			{
				const auto& l_pendingFilePathChangeData = l_pendingFilePathChangeITR->second;
				const bool  l_isFilePathChangeDirectory = l_pendingFilePathChangeITR->second.m_isDirectory ||
					                                      l_isDirectory;

				// OLD / NEW
				// REMOVED / ADDED
				// のどちらで追加されても、同FileIDなら最終的にはFilePathChangeとして扱う
				StoreFilePathChange(l_pendingFilePathChangeITR->second.m_oldFilePath, a_filePath, l_isFilePathChangeDirectory);

				m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);

				return l_isFilePathChangeDirectory;
			}

			// 対応するOld側が存在しない
			// Asset監視範囲外空新しく入ってきたFile
			// または本当の新規作成
			StoreAddChange(a_filePath, l_isDirectory);

			return l_isDirectory;
		}
		break;

		case FILE_ACTION_REMOVED:
		case FILE_ACTION_RENAMED_OLD_NAME:
		{
			// この時点では
			// Delete/Rename/Moveのどれかかくてインしていない
			// そのため少しの間だけPendingへ保持する
			const auto& l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.find(l_fileID);

			if (l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
			{
				// 同じFileIDのOld側通知が連続した
				// 通常はNew側通知を挟むはずだが
				// 通知欠落や非常に高速な連続操作では
				// 発生するかの末井がある
				// 古いPendingをDeleteとして確定してから
				// 新しいOld側をPendingとして保持する
				const bool l_previousPendingIsDirectory = l_pendingFilePathChangeITR->second.m_isDirectory;

				StoreDeleteChange(l_pendingFilePathChangeITR->second.m_oldFilePath, l_previousPendingIsDirectory);

				m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);

				PendingFilePathChangeData l_pendingFilePathChangeData = {};

				l_pendingFilePathChangeData.m_oldFilePath    = a_filePath;
				l_pendingFilePathChangeData.m_registeredTime = std::chrono::steady_clock::now();
				l_pendingFilePathChangeData.m_isDirectory    = l_isDirectory;

				m_pendingFilePathChangeDataMap.try_emplace(l_fileID, std::move(l_pendingFilePathChangeData));

				return l_previousPendingIsDirectory;
			}

			PendingFilePathChangeData l_pendingFilepathChangeData = {};

			l_pendingFilepathChangeData.m_oldFilePath    = a_filePath;
			l_pendingFilepathChangeData.m_registeredTime = std::chrono::steady_clock::now();
			l_pendingFilepathChangeData.m_isDirectory    = l_isDirectory;
			
			m_pendingFilePathChangeDataMap.try_emplace(l_fileID, std::move(l_pendingFilepathChangeData));

			return false;
		}
		break;

		default:
		break;
	}

	return false;
}
bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessExpiredPendingFilePathChange()
{
	if (m_pendingFilePathChangeDataMap.empty()) { return false; }

	const auto& l_currentTime               = std::chrono::steady_clock::now      ();
	      auto  l_pendingFilePathChangeITR  = m_pendingFilePathChangeDataMap.begin();
	      bool  l_requiresFolderTreeRefresh = false;

	while (l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
	{
		const auto& l_pendingFilePathChangeData = l_pendingFilePathChangeITR->second;

		// まだ猶予時間内なら処理を飛ばす
		if (l_currentTime - l_pendingFilePathChangeData.m_registeredTime < k_pendingFilePathChangeGracePeriod)
		{
			++l_pendingFilePathChangeITR;

			continue;
		}

		// 猶予時間内に同FileIDのNew側通知が来なかった
		// 監視範囲から完全に消えたFile/DirectorとしてDeleteへ確定する
		StoreDeleteChange(l_pendingFilePathChangeData.m_oldFilePath, l_pendingFilePathChangeData.m_isDirectory);

		l_requiresFolderTreeRefresh = l_pendingFilePathChangeData.m_isDirectory ||
			                          l_requiresFolderTreeRefresh;

		l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);
	}

	return l_requiresFolderTreeRefresh;
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StoreAddChange(const std::filesystem::path& a_filePath, const bool a_isDirectory)
{
	if (a_filePath.empty()) { return; }

	auto l_directoryChange = std::make_unique<AssetBrowserEditorWindowDirectoryAddChange>();

	// 必要な情報を格納
	l_directoryChange->SetFilePath   (a_filePath);
	l_directoryChange->SetIsDirectory(a_isDirectory);

	// 設定完了後Listへ追加
	m_directoryChangeList.emplace_back(std::move(l_directoryChange));
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StoreDeleteChange(const std::filesystem::path& a_filePath, const bool a_isDirectory)
{
	if (a_filePath.empty()) { return; }

	auto l_directoryChange = std::make_unique<AssetBrowserEditorWindowDirectoryDeleteChange>();

	// 必要な情報を格納
	l_directoryChange->SetFilePath   (a_filePath);
	l_directoryChange->SetIsDirectory(a_isDirectory);

	// 設定完了後Listへ追加
	m_directoryChangeList.emplace_back(std::move(l_directoryChange));
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StoreFilePathChange(const std::filesystem::path& a_oldFilePath, const std::filesystem::path &a_newFilePath, const bool a_isDirectory)
{
	if (a_oldFilePath.empty() ||
		a_newFilePath.empty())
	{
		return;
	}

	if (a_oldFilePath == a_newFilePath) { return; }
	
	auto l_directoryChange = std::make_unique<AssetBrowserEditorWindowDirectoryFilePathChange>();

	// 必要な情報を格納
	l_directoryChange->SetFilePath   (a_oldFilePath);
	l_directoryChange->SetNewFilePath(a_newFilePath);
	l_directoryChange->SetIsDirectory(a_isDirectory);

	// 設定完了後Listへ追加
	m_directoryChangeList.emplace_back(std::move(l_directoryChange));
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ApplyDirectoryChangeList(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
	// AddChangeがFile削除に失敗した場合
	// Retryが必要なのでvectorへ移す
	auto l_directoryChangeITR = m_directoryChangeList.begin();

	while (l_directoryChangeITR != m_directoryChangeList.end())
	{
		auto& l_directoryChange = *l_directoryChangeITR;

		if (!l_directoryChange)
		{
			l_directoryChangeITR = m_directoryChangeList.erase(l_directoryChangeITR);

			continue;
		}

		l_directoryChange->Apply(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

		// trueなら次Frameでも同じChangeをもう一度Applyする
		if (l_directoryChange->GetVALIsRequiresRetry())
		{
			++l_directoryChangeITR;

			continue;
		}

		// 正常に完了したChangeはvectorから削除する
		l_directoryChangeITR = m_directoryChangeList.erase(l_directoryChangeITR);
	}
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ResetPendingFilePathChange()
{
	m_pendingFilePathChangeDataMap.clear();
}