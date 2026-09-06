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

bool FWK::Editor::AssetBrowserEditorWindowDirectoryNotificationProcessor::ProcessExpiredPendingFilePathChange()
{
	// Pendingが1件も存在しないなら
	// Delete確定待ちやRename待ちは存在しないので何もしない
	if (m_pendingFilePathChangeDataMap.empty()) { return false; }

	const auto& l_currentTime = std::chrono::steady_clock::now();

	// unordered_mapから期限切れ要素をerase(9子ながら進む為にIteratorを使用する
	auto l_pendingFilePathChangeITR  = m_pendingFilePathChangeDataMap.begin();
	bool l_requiresFolderTreeRefresh = false;

	while (l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
	{
		const auto& l_pendingFilePathChangeData = l_pendingFilePathChangeITR->second;

		// Pendingを登録してから現在まで、
		// どれだけ時間が経過したかを計算する
		if (l_currentTime - l_pendingFilePathChangeData.m_registeredTime < k_pendingFilePathChangeGracePeriod)
		{
			// まだ期限切れではないので、
			// このPendingはMapへ残したまま次へ進む
			++l_pendingFilePathChangeITR;

			continue;
		}

		// 250msまで経過しているなら対応するNew側通知が来なかったということ
		// Removed,RenameOldNameで意味が異なるのでTypeごとに処理を分ける
		switch (l_pendingFilePathChangeData.m_type)
		{
			case PendingFilePathChangeType::Removed:
			{
				// REMOVEDの場合250ms待機しても同FileIDのADDEDが来なかったら監視範囲内Moveではなく監視対象からFile/Directoryが消えたと判断する
				// そのためDeleteChangeへ確定する
				StoreDeleteChange(l_pendingFilePathChangeData.m_oldFilePath, l_pendingFilePathChangeData.m_isDirectory);

				// Directoryが削除された場合
				// AssetBrowser左側のFolderTreeも実状態と変わるため
				// Refreshが必要になる
				l_requiresFolderTreeRefresh = l_pendingFilePathChangeData.m_isDirectory ||
					                          l_requiresFolderTreeRefresh;
			}
			break;

			case PendingFilePathChangeType::RenameOldName:
			{
				// OLD_NAMEで250ms経過NEW_NAMEが来なかったからと言って、FileがDeleteされたと判断してはいけない
				// 本当はRenameされているのにNewName通知だけ取得できなかった可能性があるため
				// NewPathが分からない状態でRegistryを書き換えると
				// 誤ったAsset情報になるのでここでは推測をしない
				FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorでRenameNewName通知を取得できませんでした。\nOldFilePath : {}", l_pendingFilePathChangeData.m_oldFilePath.string());

				// DirectoryのRenameだった場合は勿論FolderTreeを再構築する必要がある
				// Fileの場合も通知系列が壊れている異常状態のなので、
				// 現段階ではtrueを返してAssetBrowser側へ再確認が必要な状態として伝える
				l_requiresFolderTreeRefresh = true;
			}
			break;

			case PendingFilePathChangeType::Invalid:
			{
				// InvalidのPendingがMapへ残ることは
				// 正常な処理では発生しない
				// 内部状態がおかしい可能性があるため、
				// Registryを推測で変更せず警告だけ出す
				FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorにInvalid状態のPendingFilePathChangeDataが残っています。");

				l_requiresFolderTreeRefresh = true;
			}
			break;

			default:
			break;
		}

		// 今処理したPendingは期限切れなのでMapから削除する
		l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);
	}

	return l_requiresFolderTreeRefresh;
}

bool FWK::Editor::AssetBrowserEditorWindowDirectoryNotificationProcessor::ProcessNotification(const std::filesystem::path& a_filePath, const FILE_NOTIFY_EXTENDED_INFORMATION& a_notificationInformation)
{
	// 通知対処Pathが空なら
	// Changeを生成するための対象を特定できないため処理しない
	if (a_filePath.empty()) { return false; }

	// FileAttributeには
	// 通知対象がFileなのかDirectoryなのか塘路の属性がBitFlagで格納されている
	const bool l_isDirectory = (a_notificationInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != static_cast<DWORD>(NULL);

	// FILE_NOTIFY_EXTENDE_INFORMATION::FileIdは
	// fileSystemがFile/Directoryへ割り当てている識別子
	// RenameやMoveではPathが変化するため
	// Asset/Test.json -> Asset/Test2.json
	// Pathだけを比較すると別物に見える
	// しかし同一FileであればFileIdが同じ
	// OLD : FileId = 123;
	// NEW : FileId = 123;
	// という情報から同じFileのPathだけが変わったと判断するために使用する
	const auto l_fileID = static_cast<std::int64_t>(a_notificationInformation.FileId.QuadPart);

	// FileがFilesystemへ作成された時刻
	// FileIdを同一File判定の中心として使用し、
	// CreationTimeはPending側に保存していた情報と
	// 今回来た通知が本当に同じFileなのかを
	// 追加確認するために使用する
	// FileId一致、CreationTime一致も場合にOld側/New側を結び付ける
	const auto l_creationTime = static_cast<std::int64_t>(a_notificationInformation.CreationTime.QuadPart);

	// ActionにはWindowsが検出した変更種類が格納されている
	// FILE_ACTION_ADDED            : File/Directoryが追加された 
	// FILE_ACTION_REMOVED          : File/Directoryが監視Directoryから消えた
	// FILE_ACTION_RENAMED_OLD_NAME : Rename前の旧Path
	// FILE_ACTION_RENAMED_NEW_NAME : Rename後の新Path
	switch (a_notificationInformation.Action)
	{
		case FILE_ACTION_ADDED:
		{
			// ADDEDだけを見ると
			// 本当に新しくFileが追加されたか、または別フォルダからMoveされたか分からない
			// そのため同じFileIdのOld側Pendingが既に存在していないか確認する
			// 同じFileIDのOld側通知が存在する場合、
			// 新規AddではなくPath変更である可能性がある
			if (const auto& l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.find(l_fileID);
				l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
			{
				const auto& l_pendingFilePathChangeData = l_pendingFilePathChangeITR->second;

				// FileIDだけでなくCreationTime、Typeも一致するなら
				// Pending側と今回のADDED側は
				// 同じFileの通知として扱う
				if (l_pendingFilePathChangeData.m_type         == PendingFilePathChangeType::Removed &&
					l_pendingFilePathChangeData.m_creationTime == l_creationTime)
				{
					const bool l_isFilePathChangeDirectory = l_pendingFilePathChangeData.m_isDirectory ||
						                                     l_isDirectory;

					// OldPathとNewPathが揃ったので
					// Rename / Moveを共通のFilePathChangeへ変換する
					StoreFilePathChange(l_pendingFilePathChangeData.m_oldFilePath, a_filePath, l_isFilePathChangeDirectory);

					// Old側とNew側の組が完成したので、
					// Pendingはもう必要ない
					m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);

					// DirectoryのMoveならFolderTreeRefreshが必要
					return l_isFilePathChangeDirectory;
				}

				// FileIDは同じだがCreationTimeが違う場合
				// 同一Fileとして結び付けてはいけない
				// 特に以前のREMOVEEDPendingだった場合は、
				// 振りFileは本当に消えていて
				// 今回は別FileのADDEDとして扱う
				if (l_pendingFilePathChangeData.m_type == PendingFilePathChangeType::Removed)
				{
					StoreDeleteChange(l_pendingFilePathChangeData.m_oldFilePath, l_pendingFilePathChangeData.m_isDirectory);
				}
				else
				{
					// RenameOldNameなのに対応するNew側を正しく取得できない状態
					FWK_ADD_LOG(Constant::k_debugWarningColor,
						        "DirectoryNotificationProcessorでRenameOldNameとAdd通知のFile情報が一致しませんでした。\nOldFilePath : {}\nNewFilePath : {}",
						        l_pendingFilePathChangeData.m_oldFilePath.string(),
						        a_filePath.string());
				}

				// 一致しなかった古いPendingは、
				// コン位階のADDEDとは結び付けられないため削除する
				m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);
			}

			// 対応するOld側が存在しない場合、
			// 今回の通知は新規AddとしてChangeへ変換する
			StoreAddChange(a_filePath, l_isDirectory);

			return l_isDirectory;
		}
		break;

		case FILE_ACTION_REMOVED:
		{
			// REMOVEDを受け取った段階では
			// 本当にDeleteされた、別フォルダへMoveされ後から同じFileIDのADDEDが来る
			// のどちらかなのかまだわからない
			// そのため即DeleteChangeへ変換せず
			// 少しだけPendingとして保持する
			// 同じFileIDのOld側Pendingが既に残っている場合は、
			// 通常の通知系列ではない
			if (const auto& l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.find(l_fileID);
				l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
			{
				FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorで同一FileIdのOld側通知が重複しました。\nFilePath : {}", a_filePath.string());

				// 古いPendingをそのまま残して
				// 新しい通知と誤結合する方が危険なので破棄する
				m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);
			}

			PendingFilePathChangeData l_pendingFilePathChangeData = {};

			// このPendingがREMOVED由来であることを記録する
			// 後で猶予時間が切れたとき
			// Removed -> New側なし -> Delete確定
			// と判断するために必要
			l_pendingFilePathChangeData.m_type = PendingFilePathChangeType::Removed;

			// Fileが消える前のFilePath
			l_pendingFilePathChangeData.m_oldFilePath = a_filePath;

			// このPendingを登録した時間
			// ProcessExpiredPendingFilePathChange(9が
			// 250msを経過したか判断するために使用する
			l_pendingFilePathChangeData.m_registeredTime = std::chrono::steady_clock::now();

			// 後から来るNew側通知と追加整合性確認用
			l_pendingFilePathChangeData.m_creationTime = l_creationTime;
			l_pendingFilePathChangeData.m_isDirectory  = l_isDirectory;

			// Key   : FileID
			// Value : OldPath等のPending情報
			// として保存する
			// 後からADDED等が来た時FileIDからこのOld側をすぐ体験できる
			m_pendingFilePathChangeDataMap.try_emplace(l_fileID, std::move(l_pendingFilePathChangeData));

			// この時点d根はDeleteかMoveか未確定なので
			// FolderTreeRefreshもまだ要求しない
			return false;
		}
		break;

		case FILE_ACTION_RENAMED_OLD_NAME:
		{
			// RenameはWindowsから基本的に
			// OLD_NAME -> NEW_NAMEという2つの通知で届く
			// OLD_NAMEだけではNewPathがまだわからないため
			// Pendingへ保持する
			if (const auto& l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.find(l_fileID);
				l_pendingFilePathChangeITR != m_pendingFilePathChangeDataMap.end())
			{
				FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorで同一FileIdのOld側通知が重複しました。\nFilePath : {}", a_filePath.string());

				m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);
			}

			PendingFilePathChangeData l_pendingFilePathChangeData = {};

			// RenameOldname由来のPending
			// NewNameが子中田t場合も
			// Deleteされたと勝手に判断してはいけないため
			// Removedと区別して保存する
			l_pendingFilePathChangeData.m_oldFilePath    = a_filePath;
			l_pendingFilePathChangeData.m_type           = PendingFilePathChangeType::RenameOldName;
			l_pendingFilePathChangeData.m_registeredTime = std::chrono::steady_clock::now();
			l_pendingFilePathChangeData.m_creationTime   = l_creationTime;
			l_pendingFilePathChangeData.m_isDirectory    = l_isDirectory;

			m_pendingFilePathChangeDataMap.try_emplace(l_fileID, std::move(l_pendingFilePathChangeData));

			return false;
		}
		break;

		case FILE_ACTION_RENAMED_NEW_NAME:
		{
			// Rename後の新Pathを受け取った
			// FileIDを使って
			// 先ほど保存したRenameOldNamePendingを検索する
			const auto& l_pendingFilePathChangeITR = m_pendingFilePathChangeDataMap.find(l_fileID);

			if (l_pendingFilePathChangeITR == m_pendingFilePathChangeDataMap.end())
			{
				FWK_ADD_LOG(Constant::k_debugWarningColor, "DirectoryNotificationProcessorでRenameNewNameに対応するOld側通知を取得できませんでした。\nNewFilePath : {}", a_filePath.string());

				// FolderTreeについては実Directoryを再確認させる
				return true;
			}

			const auto& l_pendingFilePathChangeData = l_pendingFilePathChangeITR->second;

			// FileIDだけではなくCreationTimeも一致することを確認する
			// また、PendingがRenameOldName由来であることも確認する
			if (l_pendingFilePathChangeData.m_type         != PendingFilePathChangeType::RenameOldName ||
				l_pendingFilePathChangeData.m_creationTime != l_creationTime)
			{
				FWK_ADD_LOG(Constant::k_debugWarningColor,
					        "DirectoryNotificationProcessorでRenameOldNameとRenameNewNameの対応関係が一致しませんでした。\nOldFilePath : {}\nNewFilePath : {}",
					        l_pendingFilePathChangeData.m_oldFilePath.string(),
					        a_filePath.string());

				// このPendingを後続通知へ誤って使わないよう削除する
				m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);

				return true;
			}

			const bool l_isFilePathChangeDirectory = l_pendingFilePathChangeData.m_isDirectory ||
				                                     l_isDirectory;

			// OldPath + NewPathが揃ったので
			// RenameをFilePathChangeへ変換する
			StoreFilePathChange(l_pendingFilePathChangeData.m_oldFilePath, a_filePath, l_isFilePathChangeDirectory);

			// Renameが完成したためPendingを削除
			m_pendingFilePathChangeDataMap.erase(l_pendingFilePathChangeITR);

			return l_isFilePathChangeDirectory;
		}
		break;

		default:
		break;
	}

	return false;
}