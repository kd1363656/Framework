#include "AssetBrowserEditorWindowDirectoryWatcher.h"

FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::AssetBrowserEditorWindowDirectoryWatcher() : 
	m_changeNotificationHandle(INVALID_HANDLE_VALUE)
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

	// FindFirstChangeNotification()へ渡すPathは
	// FullPathである必要があるためAbsolutePathへ変換する
	const std::filesystem::path l_absoluteDirectoryPath = std::filesystem::absolute(a_directoryPath, l_errorCode);

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherのAbsolutePathを取得できませんでした。\nFolderPath : {}", a_directoryPath.string());

		return;
	}

	// FindFirstChangeNotificationW()とは
	// Windowsへ「このDirectory以下で指定した変更が発生したら、このHANDLEをSignaled状態にしてください」と登録する
	m_changeNotificationHandle = FindFirstChangeNotificationW(l_absoluteDirectoryPath.c_str(), TRUE, k_directoryChangeNotificationFilter);

	if (m_changeNotificationHandle != INVALID_HANDLE_VALUE) { return; }

	FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更監視を開始できませんでした。FolderPath : {} Win32ErrorCode : {}", a_directoryPath.string(), GetLastError());
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Synchronize()
{
	if (m_changeNotificationHandle == INVALID_HANDLE_VALUE) { return false; }

	// WaitForSingleObject()はWindowsHandleの状態を確認するAPI
	// 今回はTimeoutに0msを指定しているので
	// 変更なし : 待機せずWAIT_TIMEOUTを即座に返す
	// 変更有   : WAIT_OBJECT_0を返す
	const DWORD l_waitResult = WaitForSingleObject(m_changeNotificationHandle, k_noWaitMilliseconds);

	if (l_waitResult == WAIT_TIMEOUT) { return false; }

	if (l_waitResult == WAIT_FAILED)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知確認に失敗しました。Win32ErrorCode : {}", GetLastError());

		// HANDLEが正常ではない可能性があるため、
		// これ以上使用しないよう解放する
		Release();

		return false;
	}

	if (l_waitResult != WAIT_OBJECT_0) { return false; }

	// 処理がここまで継続している場合、
	// File追加、削除、リネーム、フォルダ追加、削除、リネームの
	// いずれかをAssetファイルパス以下行っている
	// FindFirstChangeNotificationW()で作ったHANDLEは、
	// 一度通知を受け取った後、
	// FindNextChangeNotification(9を呼ばなければ
	// 次の変更を監視できない
	if (!FindNextChangeNotification(m_changeNotificationHandle))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで次回の変更通知を登録できませんでした。Win32ErrorCode : {}", GetLastError());

		Release();
	}

	return true;
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Release()
{
	if (m_changeNotificationHandle == INVALID_HANDLE_VALUE) { return; }

	// FindFirstChangeNotificationW()で作成したHANDLEは、
	// 一般的なCloseHandle()ではなく
	// FindCloseChangeNotification()で閉じる
	if (!FindCloseChangeNotification(m_changeNotificationHandle))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの変更通知HANDLEを解放できませんでした。Win32ErrorCode : {}", GetLastError());
	}

	m_changeNotificationHandle = INVALID_HANDLE_VALUE;
}