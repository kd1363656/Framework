
#include "AssetBrowserEditorWindow.h"

void FWK::Editor::AssetBrowserEditorWindow::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}
void FWK::Editor::AssetBrowserEditorWindow::PostDeserialize()
{
    // Asset/以下のフォルダ階層キャッシュを構築
    // BuildFolderHierarchyMapはAssetルートから再帰的に子フォルダを走査し
    // m_folderHierarchyMap[親パス] = { 子パス1, 子パス2, ...}の形で構築する
    BuildFolderHierarchyMap(Constant::k_assetRootFolderPath);

    // FolderPaneのm_folderOpenStateMapへ未登録パスを追加
    // m_folderHierarchyMapに含まれる全フォルダパスに対して
    // FolderPaneのm_folderOpenStateMapにエントリが存在しない場合は
    // falseで追加する
    // デシリアライズで復元されたパスは既にm_folderOpenStateMapにあるため
    // ここで追加されるのは「前回のプロジェクト時には存在しなかった新規フォルダ」
    // 新規フォルダは開閉状態は閉じた状態で追加
    for (const auto& [l_parentPath, l_childPathList] : m_folderHierarchyMap)
    {
        // 親フォルダ自体も,m_folderOpenStateMapへ追加(未登録の場合のみ)
        m_folderPane.AddFolderOpenState(l_parentPath, false);

        // 子フォルダもm_folderOpenStateMapへ追加(未登録の場合のみ)
        for (const auto& l_childPath : l_childPathList)
        {
            m_folderPane.AddFolderOpenState(l_childPath, false);
        }
    }

    // Assetルートフォルダ配下のファイル / ディレクトリ変更を監視する
    // Prepare内部でReadDirectoryChangesExWによる非同期監視を塘路k数r
    // 以後毎フレームSynchronizeを呼ぶことでWindows通知を取得できる
    m_directoryWatcher.Prepare(Constant::k_assetRootFolderPath);
}

void FWK::Editor::AssetBrowserEditorWindow::Draw(EditorManager& a_editorManager)
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

    ReportActiveWindowIfMouseClicked(a_editorManager);

    Utility::IMGUIDelayedTooltip(k_thisWindowExplanationLabel);

    // 毎フレームWindows通知を取得し、Changeを適用する
    // trueの場合ディレクトリ構造が変化したためm_folderHierarchyMapを再構築する
    // Synchronize内部でSceneManager::GetInstance()が必要なため取得する
    if (auto& l_sceneManager = SceneManager::GetInstance();
        m_directoryWatcher.Synchronize(m_assetFilePathRegistry, l_sceneManager))
    {
        // ディレクトリ追加/削除/リネームがあったため
        // m_folderHierarchyMapをクリアして再構築
        // クリアしないと古いエントリが残り
        // 削除されたフォルダが表示され続ける
        m_folderHierarchyMap.clear();

        BuildFolderHierarchyMap(Constant::k_assetRootFolderPath);

        // 再構築後、新規フォルダの分だけm_folderOpenStateMapへ追加
        // 既存エントリは上書きしない他mえ開閉状態は維持される
        for (const auto& [l_parentPath, l_childPathList] : m_folderHierarchyMap)
        {
            m_folderPane.AddFolderOpenState(l_parentPath, false);

            for (const auto& l_childPath : l_childPathList)
            {
                m_folderPane.AddFolderOpenState(l_childPath, false);
            }
        }

        // 現在参照中フォルダが削除されてツリーから消えた場合
        // 一階層上の親フォルダへ戻す
        // 親もMapに存在しない場合はAssetルートへ戻す
        if (!m_currentSelectFolderPath.empty() &&
            !m_folderHierarchyMap.contains(m_currentSelectFolderPath))
        {
            const auto& l_parentPath = m_currentSelectFolderPath.parent_path();

            // 戻り先フォルダを決定
            // 親フォルダがマップに存在すれば親フォルダへ
            // そうでなければルートフォルダのAssetへ
            // AssetルートはBuildFolderHierarchyMapの開始地点のため必ずマップに存在する
            const auto& l_fallbackPath = m_folderHierarchyMap.contains(l_parentPath) ? l_parentPath : Constant::k_assetRootFolderPath;

            // 現在参照中のフォルダを戻り先に更新
            m_currentSelectFolderPath = l_fallbackPath;

            // FolderPaneの選択状態も戻り咲くフォルダへ更新する
            // ツリーノードの線t買うハイライトが戻り先フォルダに移動する
            // SelectSingleFolderの第三引数をfalseにして
            // 現在参照中パスの二重更新を避ける
            auto& l_folderSelectionState = m_folderPane.GetMutableREFSelectionState();

            l_folderSelectionState.SelectSingleFolder(l_fallbackPath, *this, false);
        }
    }

    // GetContentRegionAvail()は、現在Cursor位置から
    // Window右下までに残っている描画可能領域をPixel単位で返す
    const auto& l_availableContentRegion = ImGui::GetContentRegionAvail();

    // WindowそのものをResizeした場合、
    // 現在のこのFolderPane側ではAssetPaneが最低幅を維持できない可能性がある
    // FolderPaneを描画する前にこの現在幅を有効範囲へ補正する
    m_paneSplitter.PreparePaneSize(l_availableContentRegion);

    m_folderPane.Draw(*this);

    // FolderPaneの右側へSplitterを配置する
    m_paneSplitter.Draw(k_paneSplitterLabel, l_availableContentRegion);

    m_assetPane.Draw(*this);

    // ショートカットキー処理
    // 両Pane描画後にm_activePaneが確定しているため
    // Window側でショートカットを一元処理する
    // PaneDrawの引数リストを短縮できる(ShortcutHandlerを渡す必要がない)
    // WantTextInputがtrueの間(InputText編集中は)ショートカット無効
    // Ctrl + C / Ctrl + X / Ctrl + Vをテキスト編集に合わせるため
    // ImGui::GetIO().WantTextInputはInputTextがアクティブな間trueになる
    // ImGui::IsWindowFocused : AssetBrowserウィンドウがフォーカスされているか
    // 別のエディタウィンドウにフォーカスがあるときの誤発火を防ぐ
    if (const auto& l_io = ImGui::GetIO();
        a_editorManager.GetVALCurrentActiveWindowStaticTpeID() == AssetBrowserEditorWindow::GetREFTypeINFO().k_staticTypeID &&
        ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)                                                              &&
        !l_io.WantTextInput                                                                                                 &&
        !m_deleteConfirmState.m_isActive)
    {
        switch (m_activePane)
        {
            case Enum::AssetBrowserActivePaneType::FolderPane:
            {
                // m_activePaneに応じて対処Paneのコンテキストを渡す
                // FolderPane : 選択リスト + 操作対策フォルダ(なければAssetルート)
                // 新規フォルダ作成元・貼り付け先・リネーム対象として使う
                // GetREFOperationTargetFolderPathはconst参照を返すためコピー発生なし
                const auto& l_operationTargetFolderPath = m_folderPane.FetchVALOperationTargetFolderPath();
                const auto& l_selectionState            = m_folderPane.GetREFSelectionState             ();
                const auto& l_selectedFilePathList      = l_selectionState.GetREFSelectedFilePathList   ();

                m_shortcutHandler.HandleFolderPane(*this);
                m_shortcutHandler.Handle          (l_selectedFilePathList,
                                                   l_operationTargetFolderPath,
                                                   l_operationTargetFolderPath,
                                                   *this);
            }
            break;

            case Enum::AssetBrowserActivePaneType::AssetPane:
            {
                // AssetPaneのショートカット処理
                // 捜査対象 : 選択中ファイルの最後、なければ空パス
                // (handle内部でl_isRootFolder等の判定に使用)
                const auto& l_operationTargetFilePath = m_assetPane.FetchVALOperationTargetFilePath();
                const auto& l_selectionState          = m_assetPane.GetREFSelectionState           ();
                const auto& l_selectedFilePathList    = l_selectionState.GetREFSelectedFilePathList();

                // 捜査対象フォルダ
                // PopupDrawerの右クリックメニューと同じ挙動にするため
                // 選択中ファイルがフォルダの場合はそのフォルダ自身を捜査対象にする
                // (新規作成・貼り付け先がそのフォルダの中になる)
                // 選択中ファイルがファイルの場合はその親フォルダを捜査対象にする
                // 未選択の場合は現在参照中フォルダ、それも空なAssetルート
                std::filesystem::path l_operationTargetFolderPath = {};
                std::error_code       l_errorCode                 = {};
                
                if (!l_selectedFilePathList.empty())
                {
                    l_operationTargetFolderPath = std::filesystem::is_directory(l_operationTargetFilePath, l_errorCode) ? l_operationTargetFilePath : l_operationTargetFilePath.parent_path();
                }
                else 
                {
                    const auto& l_currentPath = m_currentSelectFolderPath;

                    l_operationTargetFolderPath = l_currentPath.empty() ? Constant::k_assetRootFolderPath : l_currentPath;
                }

                // リネーム対象
                // 選択中ファイルがある場合はそのファイル自身
                // ない場合は空パス(Handle内部でl_isSingleSelectionがfalseになるためリネーム不可)
                const auto& l_selectedFilePath = l_operationTargetFilePath;

                m_shortcutHandler.HandleAssetPane(*this);
                m_shortcutHandler.Handle         (l_selectedFilePathList, 
                                                  l_selectedFilePath,
                                                  l_operationTargetFolderPath, 
                                                  *this);
            }
            break;

            default:
            break;
        }
    }

    // 削除確認ダイアログの描画
    // Stateがアクティブな場合のみモーダルが表示される
    m_deleteConfirmPopup.Draw(*this);

    ImGui::End();
}

nlohmann::json FWK::Editor::AssetBrowserEditorWindow::Serialize()
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::AssetBrowserEditorWindow::BuildFolderHierarchyMap(const std::filesystem::path& a_folderPath)
{
    // std::filesystem::directory_iteratorでa_folderPath直下のエントリを走査する
    // is_directory()でディレクトリのみを抽出する
    // 子ディレクトリをm_folderHierarchyMap[a_folderPath]へ格納する
    std::vector<std::filesystem::path> l_childFolderList = {};
    std::error_code                    l_errorCode       = {};

    for (const auto& l_entry : std::filesystem::directory_iterator(a_folderPath, l_errorCode))
    {
        // ディレクトリ以外はスキップする
        if (!l_entry.is_directory()) { continue; }

        l_childFolderList.emplace_back(l_entry.path());
    }

    // 子フォルダをソート
    // directory_iteratorの順序は未既定(Os依存)のため
    // UI表示順を安定させるためにソートする
    std::ranges::sort(l_childFolderList, 
                      [](const auto& a_lhs, const auto& a_rhs)
                      {
                          //generic_string()でプラットフォーム共通のスラッシュ区切りを獲得
                          // 大文字小文字を区別しない比較を行う
                          const auto& l_lhsSTR = a_lhs.generic_string();
                          const auto& l_rhsSTR = a_rhs.generic_string();

                          // std::ranges::lexicographical_compareで文字列比較
                          return std::ranges::lexicographical_compare(l_lhsSTR, l_rhsSTR,
                                                                      [](const char a_lhsChar, const char a_rhsChar)
                                                                      {
                                                                          // tolowerで大文字小文字を無視して比較
                                                                          return std::tolower(static_cast<unsigned char>(a_lhsChar)) < std::tolower(static_cast<unsigned char>(a_rhsChar));
                                                                      });
                      });

    m_folderHierarchyMap[a_folderPath] = std::move(l_childFolderList);

    // 子ディレクトリに対して再帰的にBuildFolderHierarchyMapを呼ぶ
    // これでAsset/以下の全フォルダ階層が構築される
    for (const auto& l_childPath : m_folderHierarchyMap[a_folderPath])
    {
        BuildFolderHierarchyMap(l_childPath);
    }
}