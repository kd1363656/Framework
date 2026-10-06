#include "AssetBrowserEditorWindowFileOperation.h"

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Rename(const std::filesystem::path&                a_targetFilePath, 
                                                                const std::string&                          a_newName,
                                                                const AssetBrowserEditorWindowAssetCreator& a_assetCreator,
                                                                      AssetFilePathRegistry&                a_assetFilePathRegistry) const
{
    // 新しいPath = 親フォルダ / 新しい名前 + 拡張子
    const auto& l_extension   = a_targetFilePath.extension().string();
    const auto& l_newFilePath = a_targetFilePath.parent_path() / (a_newName + l_extension);

    // 名前が変わっていなければ何もしない
    // 例 : "Player.json"を名前変更モードにしたが、名前を変えずにEnterを押した場合
    //      rename()を呼ぶ必要も、Registry・Json内部名を更新する必要もない
    if (l_newFilePath == a_targetFilePath) { return; }

    // 同一名が存在する場合は番号付与する
    // 例 : "Enemy.json"が既にある状態で"Player.json"を"Enemy"へ変更 -> "Enemy1.json"
    const auto& l_resolvedNewFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_newFilePath);

    std::error_code l_errorCode = {};

    // ファイルシステム上でリネームする
    // 同じボリューム内のrenameはファイルの中身を移動せず名前だけを変更するため
    // FileIdが保たれ、WatcherからはFilePathChangeとして通知される
    std::filesystem::rename(a_targetFilePath, l_resolvedNewFilePath, l_errorCode);

    // rename()に失敗した場合、ディスク上のファイルは元の名前のまま残っている
    // ここで先へ進んでRegistryを更新すると
    // 「ディスク上は旧Path、Registry上は新Path」というズレが生まれ
    // 旧PathのJsonが未登録Json扱いになりPrefab/Sceneとして読み込めなくなるため、必ずここで中断する
    if (l_errorCode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                    "ファイルのリネームに失敗しました。\nOldFilePath : {}\nNewFilePath : {}\nErrorCode : {}",
                    a_targetFilePath.string(),
                    l_resolvedNewFilePath.string(),
                    l_errorCode.value());

        return;
    }

    // AssetFilePathRegistryのPathも更新する
    // Watcher経由でも通知されるが、即座にRegistryを更新しておく
    // (フォルダや非Jsonファイルは未登録のためfalseが返るが、それは正常動作)
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

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Delete(const std::vector<std::filesystem::path>& a_filePathList) const
{
    for (const auto& l_filePath : a_filePathList)
    {
        std::error_code l_errorCode = {};

        // remove_all()はファイル・フォルダ問わず削除する
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

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Copy(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // ClipboardへCopy操作として設定する
    a_clipboard.Apply(a_filePathList, Enum::AssetBrowserFileClipboardOperationType::Copy);
}
void FWK::Editor::AssetBrowserEditorWindowFileOperation::Cut(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard) const
{
    // ClipboardへCut操作として設定する
    a_clipboard.Apply(a_filePathList, Enum::AssetBrowserFileClipboardOperationType::Cut);
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Paste(const std::vector<std::filesystem::path>&   a_destinationFolderPathList, 
                                                               const AssetBrowserEditorWindowAssetCreator& a_assetCreator, 
                                                                     AssetBrowserEditorWindowClipboard&    a_clipboard, 
                                                                     AssetFilePathRegistry&                a_assetFilePathRegistry) const
{
    // Clipboardが空なら何もしない
    if (a_clipboard.IsEmpty()) { return; }

    // 貼り付け先フォルダが一つもなければ何もしない
    if (a_destinationFolderPathList.empty()) { return; }

    const auto l_operationType = a_clipboard.GetVALOperationType();

    // 操作種別がInvalidなら何もしない
    if (l_operationType == Enum::AssetBrowserFileClipboardOperationType::Invalid) { return; }

    const auto& l_clipboardFilePathList = a_clipboard.GetREFFilePathList();

    // 切り取りは「移動」であり、コピー + 削除で実装すると
    // コピーはWatcherに未登録Jsonとして削除され、
    // 元ファイルの削除でRegistryからもEraseされてアセットが消失する
    // rename()ならFileIdが保たれるため、WatcherがFilePathChangeとして検知し
    // RegistryのPathを付け替えてくれる(D&D移動と同じ経路)
    if (l_operationType == Enum::AssetBrowserFileClipboardOperationType::Cut)
    {
        // 移動先は1か所しか存在できないため最初の有効なフォルダへ移動する
        const auto& l_destinationFolderITR = std::ranges::find_if(a_destinationFolderPathList,
                                                                  [](const std::filesystem::path& a_destinationFolderPath)
                                                                  {
                                                                      std::error_code l_errorCode = {};
                                                                  
                                                                      return std::filesystem::is_directory(a_destinationFolderPath, l_errorCode);
                                                                  });
        
        if (l_destinationFolderITR == a_destinationFolderPathList.end())
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "切り取りの貼り付け先フォルダが見つかりませんでした。");
        
            return;
        }
        
        Move(l_clipboardFilePathList, *l_destinationFolderITR);
        
        // 移動したファイルは元の場所に存在しないためClipboardをクリアする
        a_clipboard.Clear();
        
        return;
    }

    for (const auto& l_destinationFolderPath : a_destinationFolderPathList)
    {
        std::error_code l_errorCode = {};

        // is_directory()はPathがフォルダかどうかを判定する
        // error_code版を使うことで失敗時に例外を投げずl_errorCodeへ結果を格納する
        const bool l_isDirectory = std::filesystem::is_directory(l_destinationFolderPath, l_errorCode);

        // 判定そのものに失敗した場合はログを出してスキップ
        // (先に!l_isDirectoryで弾くとエラー時もログが出ないため、エラー判定を先に行う)
        if (l_errorCode)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "ペースト処理に失敗しました。\nDestinationFolderPath : {}", l_destinationFolderPath.string());

            continue;
        }

        // フォルダでなければ貼り付け先にできないためスキップ
        if (!l_isDirectory) { continue; }

        // Clipboard内の各ファイルを貼り付け先へコピーする
        for (const auto& l_sourceFilePath : l_clipboardFilePathList)
        {
            // 貼り付け先Path = コピー先フォルダ / 元ファイル名
            // 例 : l_sourceFilePath        = "Asset/Enemy.json"
            //      l_destinationFolderPath = "Asset/Prefab"
            //      貼り付け先              = "Asset/Prefab/Enemy.json"
            auto l_destinationFilePath = l_destinationFolderPath / l_sourceFilePath.filename();

            // コピー先がコピー元の中(または自分自身)にあるかチェック
            // 例 : Source      = "Asset/NewFolder"
            //      Destination = "Asset/NewFolder/NewFolder"
            // この場合、std::filesystem::copy(recursive)は
            // コピー先(自分自身)もコピー対象になってしまい無限再帰する
            bool l_isDestinationInsideSource = false;

            auto l_parent = l_destinationFolderPath;

            // 親パスが空になるまで遡る
            // 例 : "Asset/NewFolder/Sub" -> "Asset/NewFolder" -> "Asset" -> ""
            while (!l_parent.empty())
            {
                // equivalent()はOSレベルで同じファイル/ディレクトリかを判定する
                // パスの表記揺れ(絶対/相対、スラッシュ/バックスラッシュ)を吸収する
                if (std::filesystem::equivalent(l_parent, l_sourceFilePath, l_errorCode))
                {
                    l_isDestinationInsideSource = true;

                    l_errorCode.clear();

                    break;
                }

                // equivalent()はどちらかのPathが存在しないとエラーになるため
                // 判定ごとにエラーをクリアして次の親へ進む
                l_errorCode.clear();

                l_parent = l_parent.parent_path();
            }

            // 同名が存在する場合は番号付与したPathへ貼り付ける(上書きしない)
            // 例 : "Asset/Prefab/Enemy.json"が既にある -> "Asset/Prefab/Enemy1.json"
            if (std::filesystem::exists(l_destinationFilePath, l_errorCode))
            {
                l_destinationFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_destinationFilePath);
            }

            l_errorCode.clear();

            if (l_isDestinationInsideSource)
            {
                // 自分自身の中へ貼り付ける場合は
                // std::filesystem::copy(recursive)だと無限再帰するため自前の再帰コピーを使う
                // 第3引数にはコピー先ツリーの頂点(= l_destinationFilePath)を渡し
                // 再帰の内側でも同じ頂点を引き回してスキップ判定に使う
                CopyRecursiveSkippingDestination(l_sourceFilePath, l_destinationFilePath, l_destinationFilePath);
            }
            else
            {
                // 通常の再帰コピー
                // コピー先はコピー元の外なので無限再帰しない
                // 新しいファイルとして作成されるためFileIdはコピー元と別物になり
                // WatcherからはAdd(追加)として通知される
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

            // コピーされたPrefab/Sceneを新しいUUIDでRegistryへ登録する
            // Watcherの同期はAssetBrowserEditorWindow::Drawの先頭で行われるため
            // 同じFrame内で登録しておけば、次FrameのWatcher同期時には正式なアセットとして扱われる
            // (フォルダの一部だけコピーに成功した場合も、コピーできた分は登録しておく)
            RegisterCopiedAssetList(l_sourceFilePath, 
                                    l_destinationFilePath,
                                    a_assetCreator, 
                                    a_assetFilePathRegistry);
        }
    }
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Duplicate(const std::vector<std::filesystem::path>& a_filePathList, const AssetBrowserEditorWindowAssetCreator& a_assetCreator, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    for (const auto& l_sourceFilePath : a_filePathList)
    {
        // 同じフォルダ内へ同じ名前でファイル、フォルダを複製する
        // 同名は必ず存在する(自分自身)ため番号付与したPathになる
        // 例 : "Asset/Enemy.json" -> "Asset/Enemy1.json"
        const auto& l_duplicateFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_sourceFilePath);

        std::error_code l_errorCode = {};

        // copy_options::recursiveを指定するとフォルダの場合は中身ごと複製する
        // 新しいファイルとして作成されるためFileIdはコピー元と別物になり
        // WatcherからはAdd(追加)として通知される
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

            continue;
        }

        // 複製されたPrefab/Sceneを新しいUUIDでRegistryへ登録する
        // 登録しないと次FrameのWatcher同期で未登録Jsonとして物理削除されてしまう
        RegisterCopiedAssetList(l_sourceFilePath,
                                l_duplicateFilePath, 
                                a_assetCreator,
                                a_assetFilePathRegistry);
    }
}

void FWK::Editor::AssetBrowserEditorWindowFileOperation::Move(const std::vector<std::filesystem::path>& a_sourceFilePathList, const std::filesystem::path& a_destinationFolderPath) const
{
    for (const auto& l_sourceFilePath : a_sourceFilePathList)
    {
        std::error_code l_errorCode = {};
 
        // 既に移動先フォルダの直下にある場合はスキップ
        // 例 : AssetPane空白へドロップして
        //      表示中フォルダ内のアイテムを同じフォルダへ移動しようとした場合
        // このままrenameすると同名衝突で番号付与されてしまう
        if (l_sourceFilePath.parent_path() == a_destinationFolderPath) { continue; }

        // 移動元が既に存在しない場合はスキップ
        // 「Asset/A」と「Asset/A/B」を同時選択して移動した場合
        // 先にAが移動するとA/Bの旧パスは存在しなくなる
        if (!std::filesystem::exists(l_sourceFilePath, l_errorCode)) { continue; }
 
        l_errorCode.clear();
 
        // 移動先が移動元の中(または自分自身)にある場合はスキップ
        // Asset/DataをAsset/Data/Sub へ移動すると入れ子が循環する
        // Paste()と同様に移動先の親パスを遡り、equivalent()でOSレベルの同一判定を行う
        bool l_isDestinationInsideSource = false;
 
        auto l_parent = a_destinationFolderPath;
        
        while (!l_parent.empty())
        {
            if (std::filesystem::equivalent(l_parent, l_sourceFilePath, l_errorCode))
            {
                l_isDestinationInsideSource = true;
        
                l_errorCode.clear();
        
                break;
            }
        
            l_errorCode.clear();
        
            l_parent = l_parent.parent_path();
        }
 
        if (l_isDestinationInsideSource)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                        "移動先が移動元の中にあるため、移動をスキップしました。\nSourceFilePath : {}\nDestinationFolderPath : {}",
                        l_sourceFilePath.string(),
                        a_destinationFolderPath.string());
 
            continue;
        }
 
        // 移動先パス = ドロップ先フォルダ / 移動元の名前
        // 例 : l_sourceFilePath        = "Asset/Data"
        //      a_destinationFolderPath = "Asset/Sound"
        //      移動先                  = "Asset/Sound/Data"
        auto l_destinationFilePath = a_destinationFolderPath / l_sourceFilePath.filename();
 
        // 同名が存在する場合は番号付与したパスへ移動(上書きしない)
        // 例 : "Asset/Sound"にDataがすでにある->Asset/Sound/Data1
        if (std::filesystem::exists(l_destinationFilePath, l_errorCode))
        {
            l_destinationFilePath = Utility::ResolveFilePathConflictByNumberSuffix(l_destinationFilePath);
        }
 
        // std::filesystem::renameでフォルダごと移動(中身含む)
        // 同じボリューム内ならアトミックな移動(コピー + 削除よりも高速)
        // Asset内の移動なので同じボリューム前提
        std::filesystem::rename(l_sourceFilePath, l_destinationFilePath, l_errorCode);
 
        if (l_errorCode)
        {
            FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                        "ファイルの移動に失敗しました。\nSourceFilePath : {}\nDestinationFilePath : {}\nErrorCode : {}",
                        l_sourceFilePath.string(),
                        l_destinationFilePath.string(),
                        l_errorCode.value());
        }
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

void FWK::Editor::AssetBrowserEditorWindowFileOperation::RegisterCopiedAssetList(const std::filesystem::path&                a_sourceRootPath, 
                                                                                 const std::filesystem::path&                a_copiedRootPath, 
                                                                                 const AssetBrowserEditorWindowAssetCreator& a_assetCreator, 
                                                                                       AssetFilePathRegistry&                a_assetFilePathRegistry)
{
    std::error_code l_errorCode = {};

    // ファイル単体がコピーされた場合
    // コピー先がフォルダでなければコピー元と1対1で対応するため、そのまま登録する
    if (!std::filesystem::is_directory(a_copiedRootPath, l_errorCode))
    {
        // コピーに失敗してファイルが存在しない、普通のファイルじゃない場合は登録しない
        // 登録すると存在しないPathがRegistryに残ってしまう
        // (is_directory()は存在しないPathでもエラーにならずfalseを返すため、別途確認する)
        if (!Utility::CanLoadFilePath(a_copiedRootPath, Constant::k_lowerJsonExtension)) { return; }
        
        a_assetCreator.RegisterCopiedAsset(a_sourceRootPath, a_copiedRootPath, a_assetFilePathRegistry);

        return;
    }

    // フォルダがコピーされた場合
    // コピー先フォルダ内の全Jsonについて、対応するコピー元Pathを逆算して登録する
    // 例 : SourceRoot = "Asset/Enemy"
    //      CopiedRoot = "Asset/Enemy1"
    //      "Asset/Enemy1/Boss.json" -> コピー元は "Asset/Enemy/Boss.json"
    // recursive_directory_iteratorはフォルダ内をサブフォルダまで再帰的に列挙するイテレータ
    // 列挙されるPathは渡したPathを先頭に付けた形("Asset/Enemy1/...")になる
    // error_code版のコンストラクタを使い、失敗時に例外を投げないようにする
    auto l_entryITR = std::filesystem::recursive_directory_iterator(a_copiedRootPath, l_errorCode);

    // 引数無しで構築したrecursive_directory_iteratorは「列挙の終端」を表す
    const auto& l_endEntryITR = std::filesystem::recursive_directory_iterator{};

    while (!l_errorCode &&
           l_entryITR != l_endEntryITR)
    {
        const auto& l_copiedFilePath = l_entryITR->path();

        // Json以外のファイル・フォルダは登録対象外
        // ループ末尾でincrementするためcontinueは使わない
        // (continueするとイテレータが進まず無限ループになる)
        if (l_copiedFilePath.extension() == Constant::k_lowerJsonExtension)
        {
            // コピー先PathのRoot部分をコピー元Rootへ置き換えて
            // コピー元のPathを求める
            const auto& l_sourceFilePath = Utility::ReplaceRoot(l_copiedFilePath, a_copiedRootPath, a_sourceRootPath);

            // コピー元がRegistryへ登録済みのアセットなら
            // 新しいUUIDでコピー先を登録しJson内部の情報も書き換える
            a_assetCreator.RegisterCopiedAsset(l_sourceFilePath, l_copiedFilePath, a_assetFilePathRegistry);
        }

        // 次のエントリへ進む
        // ++演算子は失敗時に例外を投げるため、error_code版のincrementを使う
        l_entryITR.increment(l_errorCode);
    }

    if (l_errorCode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor,
                    "コピーしたフォルダの走査に失敗しました。\nCopiedRootPath : {}\nErrorCode : {}",
                    a_copiedRootPath.string(),
                    l_errorCode.value());
    }
}