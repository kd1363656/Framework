#include "AssetBrowserEditorWindowDirectoryWatcher.h"

FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::AssetBrowserEditorWindowDirectoryWatcher() : 

	m_overlapped(),

	m_directoryHandle        (INVALID_HANDLE_VALUE),
	m_notificationEventHandle(nullptr),

	m_notificationProcessor(),

	m_directoryPath(),

	m_isNotificationReadPending(false)
{}
FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::~AssetBrowserEditorWindowDirectoryWatcher()
{
	Release();
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Prepare(const std::filesystem::path& a_directoryPath)
{
	// Prepare()が複数回呼ばれた場合でも
	// 前回のDirectoryHandle,EventHandle,非同期Readを残さないようにReleaseする
	Release();

	if (a_directoryPath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherへ空のDirectoryPathが渡されました。");

		return;
	}

	std::error_code l_errorCode = {};

	// 監視対象が実態にDirectoryとして存在するか確認する
	// std::errro_code版を使用することで
	// Editorを構築できるようにしている
	if (!std::filesystem::is_directory(a_directoryPath, l_errorCode) ||
		l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherの監視対象Directoryを確認できませんでした。\nDirectoryPath : {}", a_directoryPath.string());

		return;
	}

	l_errorCode.clear();

	// CreateFileW()へ渡すため
	// 実際のWindowsFileSystem上のAbsolutePathを取得する
	// 例えばAsset -> C:/Project/Assetのような絶対Pathになる
	const auto& l_absoluteDirectoryPath = std::filesystem::absolute(a_directoryPath, l_errorCode);

	if (l_errorCode)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでAbsolutePathを取得できませんでした。\nDirectoryPath : {}",a_directoryPath.string());

		return;
	}

	// NotificationProcessorでは、
	// Windowの通知に入っている相対PathとこのRootPathを結合して
	// Framework内部のAssetPathを作成する
	// そのためAbsolutePathではなく
	// 呼び出し側から受け取ったpathを保持する
	m_directoryPath = a_directoryPath;

	// Directory監視でもCreateFileW()を使用する
	// CreateFileW(開くDirectoryの絶対Path、
	//             Directory変更通知を取得するためのAccess権、
	//             他ProcessがこのDirectoryをRead/ Write / Deleteできる共有設定、
	//             Security設定、nullptrなら既定Securityを使用する。
	//             既に存在するDirectoryだけを開く設定。
	//             Directoryを開く設定 + 非同期I/を有効化する設定
	//             TemplateHandle今回は使用しないためnullptr);
	m_directoryHandle = CreateFileW(l_absoluteDirectoryPath.c_str(),
			                        FILE_LIST_DIRECTORY,
			                        FILE_SHARE_READ |
			                        FILE_SHARE_WRITE |
			                        FILE_SHARE_DELETE,
			                        nullptr,
			                        OPEN_EXISTING,
			                        FILE_FLAG_BACKUP_SEMANTICS |
			                        FILE_FLAG_OVERLAPPED,
			                        nullptr);

	if (m_directoryHandle == INVALID_HANDLE_VALUE)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor,
			        "DirectoryWatcherで監視Directoryを開けませんでした。\nDirectoryPath : {}\nWin32ErrorCode : {}",
			        a_directoryPath.string(),
			        GetLastError());

		Release();

		return;
	}

	// ReadDirectoryChangesExW()の非同期Readが完了すると、
	// OVERLAPPED::hEventへっ設定したEventがSignalじょうたいになる
	// CrateEventW(SecurityAttributes、nullptrなら既定Securityを使用する、
	//             ManualResetにするか、TRUEならResetEvent()を呼ぶまでSignal状態を維持する、
	//             作成直後をSignal状態にするか、FALSEなら最初は非Signal状態を維持する);
	m_notificationEventHandle = CreateEventW(nullptr,
		                                     TRUE,
		                                     FALSE,
		                                     nullptr);

	// CreatEventW()に失敗した場合はnullptrが返される
	// EventHandleが存在しなければ非同期Read完了を確認できな隊目
	// Directory監視を開始せず全Resourceを解放する
	if (!m_notificationEventHandle)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでNotificationEventを作成できませんでした。\nWin32ErrorCode : {}", GetLastError());

		Release();

		return;
	}

	// DirectoryHandleとNotificationEventHandleの準備が完了したので
	// windowsへ最初のDirectory変更通知Readを登録する
	//  CreateFileW()でHandleを取得下だけでは、Directory変更追加はまだ取得できない
	// PrepareNotificationRead()内部でReadDirectoryChangesExW()を呼ぶことで、
	// このDirectoryに変更が発生したらNotificationBufferへ書き込んでください
	// とWindowsへ非同期Readを登録する
	if (PrepareNotificationRead()) { return; }

	// ReadDirectoryChangesExW()の登録に失敗した場合、
	// 中途半端なHandleを残さないようすべて解放する
	Release();
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Synchronize(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager)
{
	// Prepare()が正常に完了していない場合、
	// Directory監視に必要なHandleが存在しないため何もしない
	if (m_directoryHandle == INVALID_HANDLE_VALUE ||
		!m_notificationEventHandle)
	{
		return false;
	}

	bool l_requiresFolderTreeRefresh = false;

	// ReadDirectoryChangesExW()の非同期Readが完了したか確認する
	// WaitForSingleObject(状態を確認するNotificationEventHandle,
	//                     最大待機時間);
	const auto l_waitResult = WaitForSingleObject(m_notificationEventHandle, k_noWaitMilliseconds);

	// WAIT_TIMEOUTは、NotificationEventがまだSignal状態ではない
	// つまり今Frameでは新しいWindows通知が届いていない状態
	if (l_waitResult == WAIT_TIMEOUT)
	{
		l_requiresFolderTreeRefresh = m_notificationProcessor.ProcessExpiredPendingFilePathChange();

		// 前FrameでFile削除等に失敗しRetryになったChangeは
		// 新しいWindows通知がなくても再度Applyする必要がある
		m_notificationProcessor.ApplyDirectoryChangeList(a_assetFilePathRegistry, a_sceneManager);

		return l_requiresFolderTreeRefresh;
	}

	// WaitForSingleObject()事態が失敗した場合。
	// EventHandle等の監視状態を信頼できないため
	// 現在のWatcherをReleaseして中途半端な状態を残さない
	if (l_waitResult == WAIT_FAILED)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでNotificationEventの確認に失敗しました。\nWin32ErrorCode : {}", GetLastError());

		Release();

		return true;
	}

	// Eventに対するWaitForSingleObject()では
	// 最上系としてWAIT_OBJECT_0,WAIT_TIMEOUTを想定している
	// それ以外が返された場合は想定外状態なので
	// Registryを推測で変更せず監視を終了する
	if (l_waitResult != WAIT_OBJECT_0)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで想定外のWaitResultを取得しました。\nWaitResult : {}", l_waitResult);

		Release();

		return true;
	}

	DWORD l_writtenByteSize = k_initialWrittenByteSize;

	// ReadDirectoryChangesExW()で登録していた
	// 非同期I/Oの結果を取得する
	// GetOverlappedResult(非同期Readを実行したDirectoryHandle、
	//                     ReadDirectoryChangesExW()で使用したOVERLAPPED、
	//                     WindowsがBufferへ書き込んだByte数を受け取る、
	//                     非同期I/完了まで待つか
	//                     FALSEなのでThreadを停止させて待たない);
	if (!GetOverlappedResult(m_directoryHandle,
		                     &m_overlapped,
		                     &l_writtenByteSize,
		                     FALSE))
	{
		const DWORD l_errorCode = GetLastError();

		// 非同期I/Oがまだ完全に完了していない場合
		// 今Frameでは通知Bufferへ触らず
		// 次Frameでもう一度確認する
		if (l_errorCode == ERROR_IO_INCOMPLETE)
		{
			// Windows通知とは別に、
			// 前FrameからRetryしているChangeだけは再実行する
			m_notificationProcessor.ApplyDirectoryChangeList(a_assetFilePathRegistry, a_sceneManager);

			return false;
		}

		// ERROR_IO_INCOMPLETE以外なら、
		// 今回登録ｓｈ知恵いたReadは正常継続中とは扱えない
		m_isNotificationReadPending = false;

		// Windowsがわで変更通知をすべて保持できず
		// 個々の変更を正確に列挙できなくなった場合
		// OLD_NAMEだけ取得New_NAMEを取りこぼす等があり得るためPendingを信用してはいけない
		if (l_errorCode == ERROR_NOTIFY_ENUM_DIR)
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherが全てのDirectory変更通知を取得できませんでした。\nAssetDirectoryの再同期が必要です。");

			m_notificationProcessor.ResetPendingFilePathChange();

			// 今回のReadは終了しているので
			// 次の変更を取りこぼさないよう新しいReadを登録する
			const bool l_isNotificationReadPrepared = PrepareNotificationRead();

			// 既に正常な通知から作られていたChangeは
			// 通知順を維持したままApplyする
			m_notificationProcessor.ApplyDirectoryChangeList(a_assetFilePathRegistry, a_sceneManager);

			if (!l_isNotificationReadPrepared) 
			{
				Release(); 
			}

			return true;
		}

		// その他の非同期I/OError
		// HandleやOVERLAPPEDの状態を信用して監視継続すると
		// 後続処理まで壊す可能性があるためReleaseする
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで非同期Directory通知の取得に失敗しました。\nWin32ErrorCode : {}", l_errorCode);

		Release();

		return true;
	}

	// GetOverlappedResult()が成功したので
	// 今回のReadDirectoryChangesExW()は完了した
	// 次の通知を取得するためには、
	// 後でPrepareNotificationRead()を再度呼ぶ必要がある
	m_isNotificationReadPending = false;

	// Eventは完了しているのに
	// Windowsから0Byteしか取得できなかった場合、
	// 個々の変更内容を解析することができない
	// 通知取りこぼし等の可能性があるため
	// Pendingを信用せず破棄する
	if (l_writtenByteSize == k_initialWrittenByteSize)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherのNotificationBufferから変更内容を取得できませんでした。\nAssetDirectoryの再同期が必要です。");

		m_notificationProcessor.ResetPendingFilePathChange();

		// 次のFileSystem変更を取りこぼさないよう、
		// 新しい非同期Readを先に登録する
		const bool l_isNotificationReadPrepared = PrepareNotificationRead();

		m_notificationProcessor.ApplyDirectoryChangeList(a_assetFilePathRegistry, a_sceneManager);

		if (!l_isNotificationReadPrepared)
		{
			Release();
		}

		return true;
	}

	// Watcher自身は、
	// ADDED / REMOVED / RENAMED_OLD_NAME / RENAMED_NEW_NAMEを解釈しない。
	// NotificationProcessorへ
	// 監視RootPathとWindowsが書いたByte数を渡して、
	// Add/Delete/Rename/Move判定を任せる。
	l_requiresFolderTreeRefresh = m_notificationProcessor.ProcessNotificationBuffer(m_directoryPath, l_writtenByteSize);

	// Buffer内のNew側通知を全て先に処理した後で、
	// 期限切れPendingを確認する
	l_requiresFolderTreeRefresh = m_notificationProcessor.ProcessExpiredPendingFilePathChange() ||
		                          l_requiresFolderTreeRefresh;

	// AddChange等のApply()では、
	// 不正Assetをstd::filesystem::remove()する場合がある
	// Apply()によって発生したFileSystem変更も
	// Windows通知として取得したいので、
	// ChangeをApplyする前に次のReadDirectoryChangesExW()を登録する
	const bool l_isNotificationReadPrepared = PrepareNotificationRead();

	// Bufferから生成されたChangeを
	// Windows通知順のままRegistry / SceneManagerへ反映す
	m_notificationProcessor.ApplyDirectoryChangeList(a_assetFilePathRegistry, a_sceneManager);

	if (!l_isNotificationReadPrepared)
	{
		// 次のDirectory監視を登録できなかった場合は、
		// 中途半端なWatcher状態を残さない
		Release();

		return true;
	}

	return l_requiresFolderTreeRefresh;
}

void FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::Release()
{
	// DirectoryHandleが有効な場合だけ、
	// 非同期I/Oの終了処理とHandle解放を行う
	if (m_directoryHandle != INVALID_HANDLE_VALUE)
	{
		// ReadDirectoryChangesExW()による非同期Readが
		// 現在も登録中なら、Handleを閉じる前にCancelする
		if (m_isNotificationReadPending)
		{
			// CancelIoEx(Cancel対象のI/Oを実行しているHandle、
			//            Cancel対象のOVERLAPPED、nullptrならHandle上の全I/O対象になるが、今回はこのWatcherのReadだけをCancelするため、m_overlappedを指定する);
			if (!CancelIoEx(m_directoryHandle, &m_overlapped))
			{
				const DWORD l_errorCode = GetLastError();

				// ERROR_NOT_FOUNDは、Cancelしようとした時点で既にI/Oが完了した場合などに発生する
				// つまり「Cancel対象がもう存在しない」だけなので、異常終了として扱わない
				if (l_errorCode != ERROR_NOT_FOUND)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで非同期Directory監視をCancelできませんでした。\nWin32ErrorCode : {}", l_errorCode);
				}
			}

			// CancelIoEx()を呼んだ直後でも、
			// 非同期I/Oの終了処理自体がまだWindows側で完了していない可能性がある
			// BufferやHandleを先に破棄すると、
			// Windowsがまだ使用中のMemoryへAccessする危険がある
			// そのためGetOverlappedResult()で非同期I/Oが完全に終了するまで待つ
			if (auto l_writtenByteSize = k_initialWrittenByteSize;
				!GetOverlappedResult(m_directoryHandle,
				                     &m_overlapped,
				                     &l_writtenByteSize,
				                     TRUE))
			{
				const DWORD l_errorCode = GetLastError();

				// CancelIoEx()によって正常にCancelされたI/Oは、
				// GetOverlappedResult()でERROR_OPERATION_ABORTEDになる
				if (l_errorCode != ERROR_OPERATION_ABORTED)
				{
					FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで非同期I/Oの終了確認に失敗しました。\nWin32ErrorCode : {}",l_errorCode);
				}
			}

			m_isNotificationReadPending = false;
		}

		// Directory監視用Handleを解放する
		// CloseHandle(解放するHandle);
		if (!CloseHandle(m_directoryHandle))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでDirectoryHandleを解放できませんでした。\nWin32ErrorCode : {}",GetLastError());
		}

		// CloseHandle()後に古いHandle値を残さない
		m_directoryHandle = INVALID_HANDLE_VALUE;
	}

	// NotificationEventHandleが存在していれば解放する
	if (m_notificationEventHandle)
	{
		if (!CloseHandle(m_notificationEventHandle))
		{
			FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでNotificationEventHandleを解放できませんでした。\nWin32ErrorCode : {}", GetLastError());
		}

		m_notificationEventHandle = nullptr;
	}

	// OVERLAPPEDにはぜんかいの非同期I/O状態が残るため初期化する
	m_overlapped = {};

	// 前回監視していたRootPathも破棄する
	m_directoryPath.clear();

	// NotificationProcessor側に凝っているPending,ChangeList,NotificationBufferListも初期状態へ戻す
	m_notificationProcessor.Release();

	m_isNotificationReadPending = false;

}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryWatcher::PrepareNotificationRead()
{
	// DirectoryHandleが無効、またはNotificationEventHandleが存在しない場合、
	// ReadDirectoryChangesExW(9による非同期監視を開始できない
	if (m_directoryHandle == INVALID_HANDLE_VALUE ||
		!m_notificationEventHandle)
	{
		return false;
	}

	// すでに同じDirectoryChangesExW()による非同期Readが登録されている状態で
	// 同じOVERLAPPEDを再利用すると
	// 以前の非同期I/O状態を破壊する可能性がある
	// そのため1つのWatcherにつき、
	// 同時に一つだけDirectory通知Readを登録する
	if (m_isNotificationReadPending)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherで既に非同期Directory監視が実行中です。");

		return false;
	}

	// ResetEvent(非Signal状態へ戻すEventHandle);
	if (!ResetEvent(m_notificationEventHandle))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでNotificationEventをResetできませんでした。\nWin32ErrorCode : {}", GetLastError());

		return false;
	}

	// OVERLAPPEDには前回の非同期I/O情報が格納されているため、
	// 新しいReadを登録する前に初期状態へ戻す
	m_overlapped = {};

	// ReadDirectoryChangesExW()の非同期Readが完了したときに
	// Signal状態へ変更するEventHandleをOVERLAPPEDへ設定する
	m_overlapped.hEvent = m_notificationEventHandle;

	// NotificationProcessorが所持している通知BufferへのViewを取得する
	// WindowsへDirectory変更通知の非同期Readを登録する
	// ReadDirectoryChangesExW(監視対象DirectoryのHandle、
	//                         Windowsが通知Dataを書き込むBufferの先頭Address、
	//                         Windowsが使用できるBuffer全体のByte数、
	//                         ChildDirectoryまで監視するか、TRUEなのでAssetRoot配下を全て監視する、
	//                         どの種類の変更を監視するか、FILE_NAME / DIR_NAMEだけなので、Add/Delete/Rename/Moveだけを対象にする、
	//                         同期I/Oで使用する書き込みByte数、今回はOVERLAPPEDによる非同期I/Oなのでnullptr、
	//                         非同期I/O状態を保持するOVERLAPPED、
	//                         CompletionRoutine、Event + GetOverlappedResult()方式を使用するのでnullptr、
	//                         通知情報形式、FileIdやCreationTimeも取得したいためReadDirectoryNotifyExtendedInformationを使用する);
	if (const auto& l_notificationBufferList = m_notificationProcessor.GetVALNotificationBufferList();
		!ReadDirectoryChangesExW(m_directoryHandle,
		                         l_notificationBufferList.data(),
		                         static_cast<DWORD>(l_notificationBufferList.size_bytes()),
		                         TRUE,
		                         k_directoryChangeNotificationFilter,
		                         nullptr,
		                         &m_overlapped,
		                         nullptr,
		                         ReadDirectoryNotifyExtendedInformation))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryWatcherでDirectory変更通知を登録できませんでした。\nWin32ErrorCode : {}",GetLastError());

		return false;
	}

	// ReadDirectoryChangesExW(9の登録成功した
	// ここからWindowsが非同期でDirectory変更を監視し、
	// 変更が発生するとNotificationProcessorのBufferへ通知Dataを書き込む
	m_isNotificationReadPending = true;

	return true;
}