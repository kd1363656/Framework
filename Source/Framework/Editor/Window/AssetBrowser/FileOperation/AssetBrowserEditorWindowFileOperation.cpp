#include "AssetBrowserEditorWindowFileOperation.h"

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Rename(const std::filesystem::path& a_targetFilePath, const std::string& a_newName, AssetFilePathRegistry& a_assetFilePathRegistry)
{
    // 新しいPath = 親フォルダ / 新しい名前 + 拡張子
    const auto& l_extension   = a_targetFilePath.extension().string();
    const auto& l_newFilePath = a_targetFilePath.parent_path() / (a_newName + l_extension);

    std::error_code l_errorCode = {};

    // 同一名が存在する場合は番号付与する
    // ただし自分自身と同じ名前の場合は番号付与しない
    const auto& l_resolvedNewFilePath = (l_newFilePath == a_targetFilePath) ? l_newFilePath : ResolveNameConflict(l_newFilePath);

    // ファイルシステム上でリネームする
    std::filesystem::rename(a_targetFilePath, l_resolvedNewFilePath, l_errorCode);

    if (l_errorCode)
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
            l_destinationFilePath = ResolveNameConflict(l_destinationFilePath);
        }

        // copy()はフォルダの場合は中身も再帰的にコピーする
        // l_sourceFilePath以下のファイルを全てl_destinationFilePathに階層ごとコピーする
        std::filesystem::copy(l_sourceFilePath,
                              l_destinationFilePath,
                              std::filesystem::copy_options::recursive,
                              l_errorCode);

        if (l_errorCode)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                        "ファイルの貼り付けに失敗しました。\nSourceFilePath : {}\nDestinationFilePath : {}\nErrorCode : {}",
                        l_sourceFilePath.string(),
                        l_destinationFilePath.string(),
                        l_errorCode.value());
        }
    }

    // Cut操作の場合は元ファイルを削除する
    if (l_operationType == Enum::AssetBrowserFileClipboardOperationType::Cut)
    {
        for (const auto& l_sourceFilePath : l_clipboardFilePathList)
        {
            std::error_code l_errorCode = {};

            std::filesystem::remove_all(l_sourceFilePath, l_errorCode);
        }
    }

    // 貼り付け完了後にClipboardをクリアする
    a_clipboard.Clear();
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Duplicate(const std::vector<std::filesystem::path>&a_filePathList)
{
    for (const auto& l_sourceFilePath : a_filePathList)
    {
        // 同じフォルダ内へ同じ名前でファイル、フォルダを複製する
        const auto& l_duplicateFilePath = ResolveNameConflict(l_sourceFilePath);

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

std::filesystem::path FWK::Editor::AssetBrowserEditorWindowFileOperation::ResolveNameConflict(const std::filesystem::path & a_desiredPath)
{
    std::error_code l_errorCode = {};

    // 希望するPathが存在しないならそのまま返す
    if (!std::filesystem::exists(a_desiredPath, l_errorCode)) { return a_desiredPath; }

    // ファイル名のStemと拡張子を取得
    // 例 : "Player.png" -> stem = "Player", extension = ".png"
    const auto& l_stem       = a_desiredPath.stem       ().string();
    const auto& l_extension  = a_desiredPath.extension  ().string();
    const auto& l_parentPath = a_desiredPath.parent_path();

    // Player1,Player2...と番号を増やしながら存在をチェックする
    auto l_number = k_initialNameConflictResolveNumber;

    while (true)
    {
        // stem + 番号 + 拡張子を統合した新しいPathを作る
        const auto& l_candidatePath = l_parentPath / (std::format("{}{}{}", l_stem, l_number, l_extension));

        l_errorCode.clear();

        // ファイルパスが存在しなければ他の番号と被りが発生していないため
        // そのファイルパスを返す
        if (!std::filesystem::exists(l_candidatePath, l_errorCode)) { return l_candidatePath; }
    }
}