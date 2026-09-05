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

	if (l_waitResult == WAIT_TIMEOUT) { return false; }

	if (l_waitResult == WAIT_FAILED)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知確認に失敗しました。\nWin32ErrorCode : {}", GetLastError());

		Release();

		return false;
	}

	if (l_waitResult != WAIT_OBJECT_0) { return false; }

	// ReadDirectoryChangesExW()は非同期で実行しているため
	// 実際にBufferへ何Byte書き込まれたかは
	// GetOverlappedResult()から取得する
	if (DWORD l_transferredByteSize = k_initialTransferredByteSize;
		!GetOverlappedResult(m_directoryHandle,
		                     &m_overlapped,
	                         &l_transferredByteSize,
		                     FALSE))
	{
		const DWORD l_errorCode = GetLastError();

		// Event確認と完了状態のタイミングがわずかにずれた場合は
		// つふぃFrameで再確認すればいい
		if (l_errorCode == ERROR_IO_INCOMPLETE) { return false; }

		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの非同期変更通知取得に失敗しました。\nWin32ErrorCode : {}", l_errorCode);

		Release();

		return false;
	}

	// 0Byteの場合、
	// Windows側通知BufferがOverflowして表際な変更情報を取得できなかった可能性がある
	// Microsoftの仕様上、この場合はDirectoryを再列挙して差分を再構築する必要がある
	

	return false;
}


void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Release()
{

}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::PrepareNotificationRead()
{
	return false;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessNotificationBuffer(const DWORD& a_transferredByteSize)
{
	return false;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ProcessNotification(const FILE_NOTIFY_EXTENDED_INFORMATION& a_notificationInformation)
{
	return false;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StorePendingRenameAsDelete()
{
	return false;
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StoreAddChange(const std::filesystem::path& a_filePath, const bool a_isDirectory)
{
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StoreDeleteChange(const std::filesystem::path& a_filePath, const bool a_isDirectory)
{
}
void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::StoreFilePathChange(const std::filesystem::path& a_oldFilePath, const std::filesystem::path &a_newFilePath, const bool a_isDirectory)
{
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ApplyDirectoryChangeList(AssetFilePathRegistry& a_assetBrowserAssetFilePathRegistry, SceneManager& a_sceneManager)
{
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::ResetPendingRename()
{
}