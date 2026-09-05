#include "AssetBrowserEditorWindowDirectoryWatcher.h"

FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::AssetBrowserEditorWindowDirectoryWatcher() : 
	m_directoryChangeList(),

	m_notificationBufferList(),

	m_directoryHandle        (INVALID_HANDLE_VALUE),
	m_notificationEventHandle(nullptr),

	m_overlapped(),

	m_directoryPath           (),
	m_pendingRenameOldFilePath(),

	m_pendingRenameFileID(k_initialPendingRenameFileID),

	m_isPendingRenameDirectory (false),
	m_isPendingRename          (false),
	m_isNotificationReadPending(false)
{}
FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::~AssetBrowserEditorWindowDirectoryWatcher()
{
	Release();
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Prepare(const std::filesystem::path& a_directoryPath)
{
	// Prepare()複数呼ばれても
	// 古い監視Handleを残さないように最初に開放する
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

	// Eventの状態だけ確認する
	// Timeout = 0msなのでMainThreadを停止させない
	const auto l_waitResult = WaitForSingleObject(m_notificationEventHandle, k_noWaitMilliseconds);

	// まだ通知なしの場合return
	if (l_waitResult == WAIT_TIMEOUT) { return false; }

	if (l_waitResult == WAIT_FAILED)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知確認に失敗しました。\nWin32ErrorCode : {}", GetLastError());

		Release();

		return false;
	}

	if (l_waitResult != WAIT_OBJECT_0) { return false; }

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
		if (l_errorCode == ERROR_IO_INCOMPLETE) { return false; }

		// ERROR_IO_INCOMPLETE以外なら、
		// 子の非同期read事態は完了扱いになる
		m_isNotificationReadPending = false;
		
		// Windowsが変更をすべて記録できなかった場合
		// この場合、個別の変更通知は信用できない
		if (l_errorCode == ERROR_NOTIFY_ENUM_DIR)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherが全ての変更通知を記録できませんでした。\nAssetDirectoryの再同期が必要です。");

			m_directoryChangeList.clear();

			ResetPendingRename();

			if (!PrepareNotificationRead()) { Release(); }

			// 詳細なFolder変更も失われた可能性があるので
			// FolderTreeは再構築させる
			return true;	
		}

		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの非同期変更通知取得に失敗しました。\nWin32ErrorCode : {}", l_errorCode);

		Release();

		return false;
	}

	// GetOverlappedResult()で今回のRead結果を回収できた
	m_isNotificationReadPending = false;

	// 0Byteなら通知BufferOverflow等によって
	// 詳細な変更情報が失われている
	if (l_transferredByteSize == k_initialTransferredByteSize)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知Bufferから詳細情報を取得できませんでした。\n大量のFile変更によってBufferがOverflowした可能性があります。");

		m_directoryChangeList.clear();

		ResetPendingRename();

		if (!PrepareNotificationRead())
		{
			Release();
		}

		return true;
	}

	// Windows通知Bufferを解析し、
	// AddChange / DeleteChange / FilePathChangeへ変換する
	const bool l_requiresFolderTreeRefresh = ProcessNotificationBuffer(l_transferredByteSize);

	// AddChange::Apply()が外部通知Jsonをremove()した場合、
	// そのremove()事態も新しいDirectory変更になる
	// Applyより先に次回readを登録することで
	// Change::Apply(9によって発生したFileSystem変更も取りこぼさない
	const bool l_isNotificationReadPrepared = PrepareNotificationRead();

	// Buffer内の情報はすでに各ChangeへPath等とコピー済みなので、
	// 同じBufferをWindowsへ再度渡しても問題ない
	ApplyDirectoryChangeList(a_assetBrowserAssetFilePathRegistry, a_sceneManager);

	if (!l_isNotificationReadPrepared) { Release(); }

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
				const DWORD l_errorCode = GetLastError();

				// ERROR_NOT_FOUND : Cancel対象I/Oが既に完了していた場合など
				// 以上として扱う必要はない
				if (l_errorCode != ERROR_NOT_FOUND)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの非同期I/をCancel出来ませんでした。\nWin32ErrorCode : {}", l_errorCode);
				}
			}

			DWORD l_transferredByteSize = k_initialTransferredByteSize;

			// CancelIoEx()後、
			// I/OSubsystemがCancelを正式に完了するまで待つ
			// TRUE : I/Oが完了するまでここでは待機する
			// Release時のみなので毎Frame処理には影響しない
			if (!GetOverlappedResult(m_directoryHandle,
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

	m_notificationBufferList.fill(std::byte{});

	m_directoryPath.clear      ();
	m_directoryChangeList.clear();

	ResetPendingRename();

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
	const auto l_readResult = ReadDirectoryChangesExW(m_directoryHandle,
		                                              m_notificationBufferList.data(),
		                                              static_cast<DWORD>(m_notificationBufferList.size()),
													  TRUE,
												      k_directoryChangeNotificationFilter,
												      nullptr,
													  &m_overlapped,
												      nullptr,
													  ReadDirectoryNotifyExtendedInformation);

	// ReadDirectoryChangesExW()は、
	// 非同期利用の場合も監視要求のQueue登録に成功すると
	// 非0を返す
	if (!l_readResult)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知登録に失敗しました。\nWin32ErrorCode : {}", GetLastError());

		return false;
	}

	m_isNotificationReadPending = true;

	return true;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessNotificationBuffer(const DWORD& a_transferredByteSize)
{
	const auto&       l_transferredByteSize       = static_cast<std::size_t>(a_transferredByteSize);
	      bool        l_requiresFolderTreeRefresh = false;
	      std::size_t l_bufferOffset              = k_initialBufferOffset;

	// FILE_NOTIFY_EXTENDED_INFORMATIONは
	// 最後のFileNameが可変長配列になっている。
	// offsetof()を使用すると、
	// 構造体先頭からFileName直前までのByte数を取得できる
	const auto& l_notificationFixedByteSize = offsetof(FILE_NOTIFY_EXTENDED_INFORMATION, FileName);

	while (l_bufferOffset < l_transferredByteSize)
	{
		const auto l_remainingByteSize = l_transferredByteSize - l_bufferOffset;

		// 固定部分すらBuffer内に存在しなければ
		// Windowから受け取ったDataを安全に解析できない
		if (l_remainingByteSize < l_notificationFixedByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知Bufferに不正なDataが含まれています。");

			break;
		}

		// WindowsがBufferへ直接
		// FILE_NOTIFY_EXTENDE_INFORMATION形式で書き込んでいるため
		// 現座Offsetを構造体Pointerとして解釈する
		const auto* l_notificationInformation = reinterpret_cast<const FILE_NOTIFY_EXTENDED_INFORMATION*>(m_notificationBufferList.data() + l_bufferOffset);

		// FileNameLengthはByte数
		// 固定Struct部分 + FileName部分が現在1Recordの実DataSize
		const auto l_recordByteSize = l_notificationFixedByteSize + static_cast<std::size_t>(l_notificationInformation->FileNameLength);

		if (l_recordByteSize > l_remainingByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知BufferにBuffer範囲外のFileNameLengthが含まれています。");

			break;
		}

		l_requiresFolderTreeRefresh = ProcessNotification(*l_notificationInformation) || l_requiresFolderTreeRefresh;

		// NextEntryOffset == NULLは
		// 現在RecordがBuffer最後という意味
		if (l_notificationInformation->NextEntryOffset == static_cast<DWORD>(NULL)) { return; }

		const auto l_nextEntryOffset = static_cast<std::size_t>(l_notificationInformation->NextEntryOffset);

		// 次Record位置が現在Recordより前だったりBuffer外なら節枝
		if (l_nextEntryOffset < l_recordByteSize ||
			l_nextEntryOffset > l_remainingByteSize)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの通知Bufferに無効なNextEntryOffsetが含まれています。");

			break;
		}

		l_bufferOffset += l_nextEntryOffset;
	}

	return l_requiresFolderTreeRefresh;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessNotification(const FILE_NOTIFY_EXTENDED_INFORMATION& a_notificationInformation)
{
	// FileNameLengthはByte数なので、
	// WCHARのByteSizeで割り切れなければ不正
	if (a_notificationInformation.FileNameLength == static_cast<DWORD>(NULL) ||
		a_notificationInformation.FileNameLength & sizeof(WCHAR) != static_cast<DWORD>(NULL))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherから無効なFileNameLengthを受け取りました。");

		return false;
	}

	// FilenameはNULL終端されていない
	// そのためFileNameLength / sizeof(WCHAR)で
	// 正確な文字数を求めてwstringを作成する
	const auto&         l_fileNameCharacterCount = static_cast<std::size_t>(a_notificationInformation.FileNameLength) / sizeof(WCHAR);
	const std::wstring& l_relativeFilePathString = { a_notificationInformation.FileName, l_fileNameCharacterCount };

	const auto l_filePath    = m_directoryPath / std::filesystem::path{ l_relativeFilePathString };
	const bool l_isDirectory = (a_notificationInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != static_cast<DWORD>(NULL);

	// FILE_NOTIFY_EXTENDED_INFORMATIONには
	// FileSystem上のFile/Directory識別子が含まれている
	// RenameOLD/NEWが同じFileか確認するため利用する
	const auto& l_fileID = static_cast<std::int64_t>(a_notificationInformation.FileId.QuadPart);

	switch (a_notificationInformation.Action)
	{
		case FILE_ACTION_ADDED:
		{
			StoreAddChange(l_filePath, l_isDirectory);

			return l_isDirectory;
		}
		break;

		case FILE_ACTION_REMOVED:
		{
			StoreDeleteChange(l_filePath, l_isDirectory);

			return l_isDirectory;
		}
		break;

		case FILE_ACTION_RENAMED_OLD_NAME:
		{
			// 既に前のOLD_NAMEが残っているなら、
			// 新しいRenameOLDを保持する前に
			// 古いOLDをDeleteして確定する
			const bool l_requiresFolderTreeRefresh = StorePendingRenameAsDelete();

			m_pendingRenameOldFilePath = l_filePath;
			m_pendingRenameFileID      = l_fileID;
			m_isPendingRenameDirectory = l_isDirectory;
			m_isPendingRename          = true;
			
			// このOLD自身については
			// NEW_NAMEが車でRenameが確定していないので
			// まだFolderTreeRefresh判定には含めない
			return l_requiresFolderTreeRefresh;
		}
		break;

		case FILE_ACTION_RENAMED_NEW_NAME:
		{
			// OLD/NEWのFileIDが一致していっれば
			// 同じFileまたはDirectoryのRename/Move
			if (m_isPendingRename &&
				m_pendingRenameFileID == l_fileID)
			{
				const bool l_isRenameDirectory = m_isPendingRenameDirectory || l_isDirectory;

				StoreFilePathChange(m_pendingRenameOldFilePath, l_filePath, l_isRenameDirectory);

				ResetPendingRename();

				return l_isRenameDirectory;
			}

			// OLDがない、またはFileIDが違う
			// PendingOLDが損じするならDeleteとして確定し、
			// 今回NEWはAddとして扱う
			const bool l_requiresFolderTreeRefresh = StorePendingRenameAsDelete();

			StoreAddChange(l_filePath, l_isDirectory);

			return l_requiresFolderTreeRefresh || l_isDirectory;
		}
		break;

		default:
		break;
	}

	return false;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StorePendingRenameAsDelete()
{
	if (!m_isPendingRename) { return false; }

	if (m_pendingRenameOldFilePath.empty()) 
	{
		ResetPendingRename();

		return false;
	}

	const bool l_isDirectory = m_isPendingRenameDirectory;

	// OLD_NAMEだけ存在して、
	// 対応するNEW_NAMEを取得出来なかったため
	// 「監視範囲から消えた」としてDeleteへ変換する
	StoreDeleteChange(m_pendingRenameOldFilePath, m_isPendingRenameDirectory);

	ResetPendingRename();

	return l_isDirectory;
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
	for (const auto& l_directoryChange : m_directoryChangeList)
	{
		if (!l_directoryChange) { continue; }


		l_directoryChange->Apply(a_assetBrowserAssetFilePathRegistry, a_sceneManager);
	}

	m_directoryChangeList.clear();
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ResetPendingRename()
{
	m_pendingRenameOldFilePath.clear();

	m_pendingRenameFileID = k_initialPendingRenameFileID;

	m_isPendingRenameDirectory = false;
	m_isPendingRename          = false;
}