
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

void FWK::Editor::AssetBrowserEditorWindow::Draw()
{
    if (!ImGui::Begin(k_editorName.data()))
    {
        ImGui::End();

        return;
    }

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
        ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) &&
        !l_io.WantTextInput)
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

                m_shortcutHandler.HandleFolderPane(*this);
                m_shortcutHandler.Handle          (m_folderPane.GetREFSelectedFilePathList(), l_operationTargetFolderPath, *this);
            }
            break;

            default:
            break;
        }
    }

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