#include "AssetBrowserEditorWindowFileOperation.h"

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Rename(const std::filesystem::path&                a_targetFilePath, 
                                                                const std::string&                          a_newName,
                                                                const AssetBrowserEditorWindowAssetCreator& a_assetCreator,
                                                                      AssetFilePathRegistry&                a_assetFilePathRegistry) const
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

    // JSON内部の名前情報を更新
    // Registryからファイルの種別を取得し、
    // 種別に応じてAssetCreatorへリネーム処理を委譲する
    // FileOperation自身はJson内のキー名(PrefabName/SceneName)を知らないため
    // AssetCreatorが各シリアライザ経由で更新する
    // フォルダや非JSONファイルはJSON内部名を持たないためスキップ
    if (l_resolvedNewFilePath.extension() != Constant::k_lowerJsonExtension) { return; }

    const auto* l_uuid = a_assetFilePathRegistry.FindPTRAssetUUID(l_resolvedNewFilePath);

    if (!l_uuid) { return; }

    const auto* l_assetFilePathData = a_assetFilePathRegistry.FindPTRAssetFilePathData(*l_uuid);

    if (!l_assetFilePathData) { return; }

    // AssetCreator::RenamePrefab/RenameSceneは既存UUIDを保持したままJSON内の名前だけ更新する
    switch (l_assetFilePathData->m_type)
    {
        case Enum::AssetFilePathRegistryType::Prefab:
        {
            a_assetCreator.RenamePrefab(l_resolvedNewFilePath, l_resolvedNewFilePath);
        }
        break;

        case Enum::AssetFilePathRegistryType::Scene:
        {
            a_assetCreator.RenameScene(l_resolvedNewFilePath, l_resolvedNewFilePath);
        }
        break;

        default:
        break;
    }
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
        // 貼り付け先Path = コピー先フォルダ/元ファイル名
        auto l_destinationFilePath = a_destinationFolderPath / l_sourceFilePath.filename();
     
        std::error_code l_errorCode = {};
     
        // コピー先がコピー元の中(または自分自身)にあるかチェック
        // 例: Source      = "Asset/NewFolder"
        //     Destination = "Asset/NewFolder/NewFolder"
        // この場合、std::filesystem::copy(recursive)は
        // コピー先(自分自身)もコピー対象になってしまい無限再帰する
        bool l_isDestinationInsideSource = false;
     
        // 親パスが空になるまで処理を実行する
        for (auto l_parent = a_destinationFolderPath; !l_parent.empty(); l_parent = l_parent.parent_path())
        {
            // equivalent()はOSレベルで同じファイル/ディレクトリかを判定する
            // パスの表記揺れ(絶対/相対、スラッシュ/バックスラッシュ)を吸収する
            if (std::filesystem::equivalent(l_parent, l_sourceFilePath, l_errorCode))
            {
                l_isDestinationInsideSource = true;

                l_errorCode.clear();

                break;
            }

            l_errorCode.clear();
        }
     
        // 同名が存在する場合は番号付与したPathへ貼り付ける(上書きしない)
        // 例: Asset/NewFolder を Asset に貼り付け、Asset/NewFolderが既にある -> Asset/NewFolder1 (兄弟番号付与)
        // 内部へ貼り付ける場合も頂点のPathを番号付与する
        if (std::filesystem::exists(l_destinationFilePath, l_errorCode))
        {
            l_destinationFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_destinationFilePath);
        }

        if (l_isDestinationInsideSource)
        {
            // 自分自身の中へ貼り付ける
            // std::filesystem::copy(recursive)はコピー先がコピー元の中にあると
            // 無限再帰するため、自前の再帰コピー関数を使う
            // 各レベルでコピー先ツリーの頂点と一致するエントリをスキップして爆発を防ぐ
            // 第3引数にはコピー先ツリーの頂点(= l_destinationFilePath)を渡す
            // 再帰の内側でも同じ頂点を引き回してスキップ判定に使う
            // なお頂点は上で番号付与済みなのでCopyRecursiveSkippingDestination内では
            // 頂点が存在することはなくcreatedirectoriesで新規作成される
            CopyRecursiveSkippingDestination(l_sourceFilePath, l_destinationFilePath, l_destinationFilePath);
        }
        else
        {
            // 通常の再帰コピー
            // コピー先はコピー元の外なので無限再帰しない
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

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Move(const std::filesystem::path& a_sourceFilePath, const std::filesystem::path& a_destinationFolderPath) const
{
    // ドロップ先フォルダの中へ移動する
    // 移動先パス = ドロップ先フォルダ / ドラッグ元フォルダ名
    // 例 : a_sourceFilePath        = "Asset/Data"
    //      a_destinationFolderPath = "Asset/Sound"
    //      -> 移動先 = "Asset/Sound/Data"
    auto l_destinationFilePath = a_destinationFolderPath / a_sourceFilePath.filename();

    std::error_code l_errorCode = {};

    // 同名が存在する場合は番号付与したパスへ移動(上書きしない)
    // 例 : "Asset/Sound"にDataがすでにある->Asset/Sound/Data1
    if (std::filesystem::exists(l_destinationFilePath, l_errorCode))
    {
        l_destinationFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_destinationFilePath);
    }

    // std::filesystem::renameでフォルダごと移動(中身含む)
    // 同じボリューム内ならアトミックな移動(コピー + 削除よりも高速)
    // Asset内の移動なので同じボリューム前提
    std::filesystem::rename(a_sourceFilePath, l_destinationFilePath, l_errorCode);

    if (l_errorCode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                    "ファイルの移動に失敗しました。\nSourceFilePath : {}\nDestinationFilePath : {}\nErrorCode : {}",
                    a_sourceFilePath.string(),
                    l_destinationFilePath.string(),
                    l_errorCode.value());
    }
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::CopyRecursiveSkippingDestination(const std::filesystem::path& a_source, const std::filesystem::path& a_destination, const std::filesystem::path& a_topDestination)
{
    // コピー先がコピー元の中(または自分自身)にある場合の再帰コピー
    // std::filesystem::copy(recursive)はコピー先を自動作成してから
    // コピー元を再帰列挙するため、コピー先がコピー元の中にあると無限再帰する
    // これを防ぐため各ディレクトリレベルで自分で列挙し
    // コピー先ツリーの頂点(a_topDestination)と一致するエントリをスキップしながらコピーする
    std::error_code l_errorCode = {};

    // コピー先ディレクトリを作成する前に
    // コピー元のエントリ一覧をスナップショットしておく
    // 作成後に列挙すると、今作ったコピー先がエントリに混ざってしまうため
    // また過去の貼り付けでコピー元の中に出来たコピー先ツリーも
    // スナップショットの時点で存在するのでスキップ判定で除外する
    std::vector<std::filesystem::path> l_sourceEntryPathList = {};

    for (const auto& l_entry : std::filesystem::directory_iterator(a_source))
    {
        l_sourceEntryPathList.emplace_back(l_entry.path());
    }

    // コピー先ディレクトリを作成(既存なら何もしない)
    // 頂点呼び出しではPaste()側で番号付与済みなので存在しない
    // 再帰の内側では既存フォルダへマージ(上書き)する
    std::filesystem::create_directories(a_destination, l_errorCode);

    if (l_errorCode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, 
                    "ファイルの貼り付け(フォルダ作成)に失敗しました。\nDestinationFilePath : {}\nErrorCode : {}",
                    a_destination.string(),
                    l_errorCode.value());

        return;
    }

    // スナップショットした各エントリをコピーする
    for (const auto& l_sourceEntryPath : l_sourceEntryPathList)
    {
        // コピー先ツリーの頂点(a_topDestination)自身はスキップする
        // a_topDestinationは外側の呼び出しで既に作成済みであり
        // コピー元ツリーの中に存在するため、これをコピーすると無限再帰する
        // 例: Asset/FolderをAsset/Folder/Folderへ貼り付けたあと
        //     Asset/FolderをAsset/Folder/Folderへ再度貼り付けると
        //     Asset/Folderの中にAsset/Folder/Folder が既にあり
        //     その中に Asset/Folder/Folder/Folder が作られる
        //     この頂点をスキップしないと永遠に再帰が深くなってしまう
        // equivalent()でOSレベルの同一判定を行いパスの表記揺れを吸収する
        if (std::filesystem::equivalent(l_sourceEntryPath, a_topDestination, l_errorCode))
        {
            l_errorCode.clear();

            continue; 
        }

        l_errorCode.clear();

        const auto& l_entryDestination = a_destination / l_sourceEntryPath.filename();

        if (std::filesystem::is_directory(l_sourceEntryPath))
        {
            // ディレクトリなら再帰(コピー先ツリーの頂点はそのまま引き継ぐ)
            CopyRecursiveSkippingDestination(l_sourceEntryPath, l_entryDestination, a_topDestination);
        }
        else
        {
            // ファイルならコピー(上書き)
            std::filesystem::copy_file(l_sourceEntryPath,
                                       l_entryDestination,
                                       std::filesystem::copy_options::overwrite_existing,
                                       l_errorCode);

            if (l_errorCode)
            {
                FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                            "ファイルの貼り付けに失敗しました。\nSourceFilePath : {}\nDestinationFilePath : {}\nErrorCode : {}",
                            l_sourceEntryPath.string(),
                            l_entryDestination.string(),
                            l_errorCode.value());
            }
        }
    }
}