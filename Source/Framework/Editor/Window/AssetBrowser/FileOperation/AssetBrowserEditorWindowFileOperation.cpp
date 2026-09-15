#include "AssetBrowserEditorWindowFileOperation.h"

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Rename(const std::filesystem::path& a_targetFilePath, const std::string& a_newName, AssetFilePathRegistry& a_assetFilePathRegistry)
{
    // 新しいPath = 親フォルダ / 新しい名前 + 拡張子
    const auto& l_extension   = a_targetFilePath.extension().string();
    const auto& l_newFilePath = a_targetFilePath.parent_path() / (a_newName + l_extension);

    std::error_code l_errorCode = {};

    // 同一名が存在する場合は番号付与する
    // ただし自分自身と同じ名前の場合は番号付与しない
    const auto& l_resolvedNewFilePath = (l_newFilePath == a_targetFilePath) ? l_newFilePath : Utility::ResolveFilePathConflictByNumberSuffix(l_newFilePath);

    // ファイルシステム上でリネームする
    std::filesystem::rename(a_targetFilePath, l_resolvedNewFilePath, l_errorCode);

    if (l_errorCode &&
        a_targetFilePath == l_newFilePath)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugINFOColor,
                    "ファイルのリネームは取り消しました。\nOldFilePath : {}\nNewFilePath : {}\nErrorCode : {}",
                    a_targetFilePath.string(),
                    l_resolvedNewFilePath.string(),
                    l_errorCode.value());
    }
    else if (l_errorCode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                    "ファイルのリネームに失敗しました。\nOldFilePath : {}\nNewFilePath : {}\nErrorCode : {}",
                    a_targetFilePath.string(),
                    l_resolvedNewFilePath.string(),
                    l_errorCode.value());   
    }

    // AssetFilePathRegistryのPathも更新する
    // Watcher経由でも通知されるが、即座にRegistryを更新しておく
    a_assetFilePathRegistry.ReplaceFilePath(a_targetFilePath, l_resolvedNewFilePath);
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Delete(const std::vector<std::filesystem::path>& a_filePathList)
{
    for (const auto& l_filePath : a_filePathList)
    {
        std::error_code l_errorCode = {};

        // remove_all()はファイル・フォルダ問わず作成する
        // フォルダの場合は中身も再帰的に削除する
        if (!std::filesystem::remove_all(l_filePath, l_errorCode) &&
            l_errorCode)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, 
                        "ファイルの削除に失敗しました。\nFilePath : {}\nErrorCode : {}", 
                        l_filePath.string(),
                        l_errorCode.value());
        }
    }
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Copy(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard)
{
    // ClipboardへCopy操作として設定する
    a_clipboard.Apply(a_filePathList, Enum::AssetBrowserFileClipboardOperationType::Copy);
}
void FWK::Editor::AssetBrowserEditorWindowFileOperation::Cut(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard)
{
    // ClipboardへCut操作として設定する
    a_clipboard.Apply(a_filePathList, Enum::AssetBrowserFileClipboardOperationType::Cut);
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Paste(const std::filesystem::path& a_destinationFolderPath, AssetBrowserEditorWindowClipboard& a_clipboard)
{
    // Clipboardが空なら何もしない
    if (a_clipboard.IsEmpty()) { return; }

    const auto l_operationType = a_clipboard.GetVALOperationType();

    // 走査種別がInvalidなら何もしない
    if (l_operationType == Enum::AssetBrowserFileClipboardOperationType::Invalid) 
    {
        return; 
    }

    const auto& l_clipboardFilePathList = a_clipboard.GetREFFilePathList();

    // Clipboard内の各ファイルを貼り付け先へコピーする
    // 同名衝突時は貼り付け側に番号付与する
    for (const auto& l_sourceFilePath : l_clipboardFilePathList)
    {
        // 貼り付け先Path = コピー先フォルダ / 元ファイル名
        auto l_destinationFilePath = a_destinationFolderPath / l_sourceFilePath.filename();

        std::error_code l_errorCode = {};

        // 同名が存在する場合は番号付与したPathへ貼り付ける
        if (std::filesystem::exists(l_destinationFilePath, l_errorCode))
        {
            l_destinationFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_destinationFilePath);
        }

        // コピー先がコピー元の中にあるかチェック
        // 例 : Source      = "Asset/NewFolder"
        //      Destination = "Asset/NewFolder/NewFolder"
        // この場合、std::filesystem::cop(recursive)は
        // コピー先(自分自身)もコピー対象なってしまい無限再帰する
        // これを防ぐため自分でディレクトを作成してから
        // 中身をコピーする際にコピー先自身をスキップする
        bool l_isDestinationInsideSource = false;

        // a_destinationFolderPathの祖先を巡り
        // l_sourceFilePathと一致するかチェック
        for (auto l_parent = a_destinationFolderPath; !l_parent.empty(); l_parent = l_parent.parent_path())
        {
            if (l_parent != l_sourceFilePath) { continue; }
            
            l_isDestinationInsideSource = true;

            break;
        }

        // コピー先ディレクトを先に作成する
        // std::filesystem::copy(recursive)は内部でディレクトリを作るが
        // 自前で再帰コピーする場合は先に作成しておく必要がある
        if (std::filesystem::is_directory(l_sourceFilePath, l_errorCode))
        {
            std::filesystem::create_directory(l_destinationFilePath, l_errorCode);
            
            if (l_errorCode) 
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                            "貼り付け先フォルダの作成に失敗しました。\nDestinationFilePath : {}\nErrorCode : {}", 
                            l_destinationFilePath.string(), 
                            l_errorCode.value());

                continue;
            }
        }

        // ディレクトリエントリーを再帰的にコピー
        // std::filesystem::directory_iteratorでsource直下のエントリを走査
        // 各エントリをdestinationへコピーする
        // コピー先がコピー元の中にある場合はコピー先自身をスキップする
        for (const auto& l_entry : std::filesystem::directory_iterator(l_sourceFilePath, l_errorCode))
        {
            const auto& l_entrySourcePath = l_entry.path                                      ();
            const auto& l_entryDestPath   = l_destinationFilePath / l_entrySourcePath.filename();

            // コピー先がコピー元の中にある場合、
            // コピー先自身(= l_destinationFilePath)と一致するエントリはスキップ
            // これにより無限再帰を防ぐ
            if (l_isDestinationInsideSource &&
                l_entrySourcePath == l_destinationFilePath)
            {
                continue;
            }

            // copy()はフォルダの場合は中身も再帰的にコピーする
            // l_entrySourcePath以下のファイルをすべてl_entryDestPathに階層ごとコピーする
            std::filesystem::copy(l_entrySourcePath,
                                  l_entryDestPath,
                                  std::filesystem::copy_options::recursive,
                                  l_errorCode);

            if (!l_errorCode)
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                            "ファイルの貼り付けに失敗しました。\nSourceFilePath : {}\nDestinationFilePath : {}\nErrorCode : {}",
                            l_entrySourcePath.string(),
                            l_entryDestPath.string(),
                            l_errorCode.value());

                l_errorCode.clear();
            }
        }
    }

    // Cut操作の以外はここで処理を終わる
    if (l_operationType != Enum::AssetBrowserFileClipboardOperationType::Cut) { return; }

    // クリップボードにコピーしたコピー元ファイルを削除する
    for (const auto& l_sourceFilePath : l_clipboardFilePathList)
    {
        std::error_code l_errorCode = {};

        std::filesystem::remove_all(l_sourceFilePath, l_errorCode);
    }

    // 貼り付け完了後にClipboardをクリアする
    a_clipboard.Clear();
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Duplicate(const std::vector<std::filesystem::path>&a_filePathList)
{
    for (const auto& l_sourceFilePath : a_filePathList)
    {
        // 同じフォルダ内へ同じ名前でファイル、フォルダを複製する
        const auto& l_duplicateFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_sourceFilePath);

        std::error_code l_errorCode = {};

        std::filesystem::copy(l_sourceFilePath,
                              l_duplicateFilePath,
                              std::filesystem::copy_options::recursive,
                              l_errorCode);

        if (l_errorCode)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                        "ファイルの複製に失敗しました。\nSourceFilePath : {}\nDuplicateFilePath : {}\nErrorCode : {}",
                        l_sourceFilePath.string(),
                        l_duplicateFilePath.string(),
                        l_errorCode.value());
        }
    }
}
