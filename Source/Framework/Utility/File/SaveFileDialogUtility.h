#pragma once

namespace FWK::Utility
{
	inline bool SaveFileDialog(const std::filesystem::path& a_initialDirectoryPath,
							   const std::wstring_view&     a_titleLabel,
		                       const std::wstring_view&     a_filterLabel,
		                       const std::wstring_view&     a_filterPatternLabel,
							   const std::wstring_view&     a_defaultExtension, 
		                             std::string&           a_filePath)
	{
		// Windowsの新しいファイル選択ダイアログを生成する
		// IFileOpenDialog -> 既存ファイルを開くためのダイアログ
		// IFileSaveDialog -> 新しいファイル名を入力して保存するためのダイアログ
		TypeAlias::ComPtr<IFileSaveDialog> l_fileDialog = nullptr;

		if (FAILED(CoCreateInstance(CLSID_FileSaveDialog,
			                        nullptr,
			                        CLSCTX_INPROC_SERVER, 
			                        IID_PPV_ARGS(&l_fileDialog))))
		{
			return false;
		}

		// ダイアログ上部に表示するタイトルを設定する
		if (FAILED(l_fileDialog->SetTitle(a_titleLabel.data()))) { return false; }

		// 保存可能なファイル形式を設定する
		// 例えば
		// a_filterLabel        = L"JSON File(*.json)"
		// a_filterPatternLabel = L"*.json"とした場合、
		// SaveDialogではJSONファイル保存対象として表示する
		const std::array<COMDLG_FILTERSPEC, Constant::k_defaultSaveFileDialogCount> l_fileTypeList = { {a_filterLabel.data(), a_filterPatternLabel.data()} };

		if (FAILED(l_fileDialog->SetFileTypes(static_cast<UINT>(l_fileTypeList.size()), l_fileTypeList.data()))) { return false; }
		
		// ユーザーがTestSceneとだけ入力した場合に、
		// TestScene.jsonのように拡張子を自動的につける
		if (FAILED(l_fileDialog->SetDefaultExtension(a_defaultExtension.data()))) { return false; }

		// 現在設定されているダイアログOptionを取得する
		FILEOPENDIALOGOPTIONS l_dialogOptions = {};

		if (FAILED(l_fileDialog->GetOptions(&l_dialogOptions))) { return false; }

		// FOS_FORCEFILESYSTEM -> 実際のWindowsファイルシステム上のPathだけを対象にする
		// FOS_PATHMUSTEXIST   -> 保存先Folderが存在している必要がある
		// FOS_OVERWRITEPROMPT -> 既存ファイルを指定した場合、「上書きしますか？」という確認をwindows側に表示する
		l_dialogOptions |= FOS_FORCEFILESYSTEM |
			               FOS_PATHMUSTEXIST   |
			               FOS_OVERWRITEPROMPT;

		if (FAILED(l_fileDialog->SetOptions(l_dialogOptions))) { return false; }

		// 保存ダイアログを最初に表示するFolderを設定する
		if (!a_initialDirectoryPath.empty())
		{
			std::error_code l_errorCode = {};

			const auto& l_initialDirectoryPath = std::filesystem::absolute(a_initialDirectoryPath, l_errorCode);

			if (l_errorCode) { return false; }

			// 初期Folderとして指定されたPathが、
			// 実際に存在するFolderか確認する
			if (!std::filesystem::is_directory(l_initialDirectoryPath, 
				                               l_errorCode) ||
				                               l_errorCode)
			{
				return false;
			}

			// std::filesystem::pathを
			// WindowShellが扱うIShellItemへ変換する
			TypeAlias::ComPtr<IShellItem> l_initialDirectoryItem = nullptr;

			if (FAILED(SHCreateItemFromParsingName(l_initialDirectoryPath.c_str(), nullptr, IID_PPV_ARGS(&l_initialDirectoryItem)))) { return false; }
		}

		// 実際に保存ダイアログを表示する
		// ユーザーがキャンセルした場合もFALSEとなるため、
		// その場合は保存処理をおこなわずfalseを返す
		if (FAILED(l_fileDialog->Show(nullptr))) { return false; }

		// ユーザーが保存ダイアログで選択した結果を取得する
		// IShellItemは、WindowsShell上のファイルやFolderを表すオブジェクト
		TypeAlias::ComPtr<IShellItem> l_selectedItem = nullptr;

		if (FAILED(l_fileDialog->GetResult(&l_selectedItem))) { return false; }

		// 選択されたIShellItemから、
		// 実際のWindowsファイルシステム上のPathを取得する
		PWSTR l_selectedFilePath = nullptr;

		if (FAILED(l_selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &l_selectedFilePath))) { return false; }

		// GetDisplayName()で取得したPWSTRは
		// Windows側が確保したメモリなので、
		// 最後にCoTaskMemFree()で解放する必要がある
		// ただし解放する前にstd::filesystem::pathへコピーする
		std::filesystem::path l_filePath = l_selectedFilePath;

		CoTaskMemFree(l_selectedFilePath);

		      std::error_code l_errorCode   = {};
		const auto&           l_currentPath = std::filesystem::current_path(l_errorCode);

		if (l_errorCode) { return false; }

		const auto& l_relativeFilePath = std::filesystem::relative(l_filePath, l_currentPath, l_errorCode);

		if (l_errorCode) { return false; }

		// 最終的に取得した保存先を呼びだし元へ返す
		a_filePath = l_relativeFilePath.string();

		return true;
	}
}