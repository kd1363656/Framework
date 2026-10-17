#include "AssetBrowserEditorWindowDeleteConfirmPopup.h"

void FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::Request(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindow& a_editorWindow) const
{
    auto& l_deleteConfirmState = a_editorWindow.GetMutableREFDeleteConfirmState();

    // 削除対象のファイルパスリストを構築
    // 選択されたパスがフォルダの場合はその中身を再帰的に展開して追加する
    // Asset/Dataを選択して削除する場合
    // Asset/Data/Dataの両方が表示される
    // これによりユーザーは何が削除されるの化が一目で分かる
    l_deleteConfirmState.m_filePathList.clear();

    std::error_code l_errorCode = {};

    for (const auto& l_filePath : a_filePathList)
    {
        // 選択されたファイルパス自身を追加
        l_deleteConfirmState.m_filePathList.emplace_back(l_filePath);

        // フォルダの場合は中身を再帰的に展開して追加
        if (std::filesystem::is_directory(l_filePath, l_errorCode))
        {
            CollectFilePathRecursive(l_filePath, l_deleteConfirmState.m_filePathList);
        }
    }

    // 確認ダイアログをアクティブにする
    // キーで選ぶボタンは、ダイアログが開いた最初のフレームにEditorPopupNavigatorが削除へ戻す
    l_deleteConfirmState.m_isActive = true;

    // OpenPopup要求フラグを立てる
    // Requestは別ポップアップ内から呼ばれる可能性があるため
    // ここではOpenPopupを呼ばずDraw側で呼ぶ
    l_deleteConfirmState.m_isOpenRequested = Constant::k_initialAssetBrowserDeleteConfirmIsOpenRequested;
}

void FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::Draw(AssetBrowserEditorWindow& a_editorWindow) const
{
    auto& l_deleteConfirmState = a_editorWindow.GetMutableREFDeleteConfirmState();

    // アクティブ出ない場合は何もしない
    if (!l_deleteConfirmState.m_isActive) { return; }

    bool l_isOpenPopup = false;

    // 初回のみPopupを呼ぶ
    // 毎フレーム呼ぶとポップアップが感じても再オープンされてしまうため
    // フラグで初回のみ制御する
    if (l_deleteConfirmState.m_isOpenRequested)
    {
        // ImGui::OpenPopup : 指定IDのポップアップを次のBeginPopupModalで開く
        // 同じImGui::Begin / Endで呼ぶ必要がある
        ImGui::OpenPopup(k_titleLabel.data());

        l_deleteConfirmState.m_isOpenRequested = false;
        l_isOpenPopup = true;
    }

    // キャンセルボタンの横幅を事前に決める
    // Buttonの横幅は
    // テキスト幅 + 左側FramePadding.x + 右側FramePadding.xとなる
    const float l_cancelButtonWidth = ImGui::CalcTextSize   (k_cancelLabel.data()).x + ImGui::GetStyle().FramePadding.x * k_framePaddingBothSidesNUM;
    const auto* l_mainViewport      = ImGui::GetMainViewport();

    if (!l_mainViewport)
    {
        if (l_isOpenPopup)
        {
            ImGui::EndPopup();
        }

        return;
    }

    // Popupが最初に表示されたときだけ
    // メインViewportの中央絵へ配置する
    // Pivot(0.5F, 0.5F)を指定することで
    // Popup自身の中央がViewport中央へ配置される
    // ImGuiCond_Appearingなので
    // 表示後にユーザーがPopupをドラッグして移動することは可能
    ImGui::SetNextWindowPos(l_mainViewport->GetCenter(), ImGuiCond_Appearing, k_modalPopupPivot);

    // Popupの横幅のみ固定する
    // 最小幅と最大幅を同じk_popupWidthにすることで
    // 横幅は常に固定される
    // 高さは0.0F ~ FLT_MAXとし
    // BeginPopupModalのAlwaysAutoResizeによって
    // 内容に合わせて自動調整する
    ImGui::SetNextWindowSizeConstraints(ImVec2{ k_popupWidth, Constant::k_imguiRemainingSize.y}, ImVec2{k_popupWidth, FLT_MAX});

    // モーダルPopupを開始する
    // モーダルPopup表示中は背後のEditorWindowを操作できなくなる
    if (!ImGui::BeginPopupModal(k_titleLabel.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Popupが何らかの理由で閉じられた場合は
        // State側も非アクティブ戻す
        l_deleteConfirmState.m_isActive = false;

        return;
    }

    // 左右キーとマウスでボタンを選べるようにする
    // 開いた直後は削除(左のボタン)がキーで選ばれた状態から始める
    auto& l_popupNavigator = l_deleteConfirmState.m_popupNavigator;

    l_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Horizontal, k_initialKeySelectedButtonIndex);

    // 削除確認メッセージを表示
    ImGui::TextUnformatted(k_messageLabel.data());
    ImGui::Spacing        ();

    // 削除対象ファイルリスト描画
    DrawFileList(l_deleteConfirmState);

    ImGui::Spacing();

    // 削除/キャンセルボタン描画
    // DrawButtonsは描画結果をButtonDrawResult構造体で値返しする
    const auto l_buttonDrawResult = DrawButtons(l_cancelButtonWidth, l_deleteConfirmState);

    l_popupNavigator.End();

    // Enter/クリックで確定
    HandleConfirm(l_buttonDrawResult, a_editorWindow, l_deleteConfirmState);

    ImGui::EndPopup();
}

void FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::DrawFileList(Struct::AssetBrowserEditorWindowDeleteConfirmState& a_deleteConfirmState) const
{
    // 削除対象の一覧をスクロール可能なChildWindowとして表示する
    // 空に0.0Fを指定すると親Windowのコンテンツ領域の残り幅いっぱいまで使用する
    if (const ImVec2& l_childWindowSize = { Constant::k_imguiRemainingSize.x, k_fileListChildHeight };
        ImGui::BeginChild(k_childLabel.data(), l_childWindowSize, true))
    {
        for (const auto& l_filePath : a_deleteConfirmState.m_filePathList)
        {
            // generic_stringを使用して
            ImGui::TextUnformatted(l_filePath.generic_string().c_str());
        }
    }

    ImGui::EndChild();
}
FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::ButtonDrawResult FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::DrawButtons(const float a_cancelButtonWidth, Struct::AssetBrowserEditorWindowDeleteConfirmState& a_deleteConfirmState) const
{
    ButtonDrawResult l_result = {};

    // ボタン行を描画し始める地点のX座標を保持する
    // この位置はPopupのコンテンツ領域左端になる
    const float l_buttonRowStartX = ImGui::GetCursorPosX();

    // 現在位置から使用可能なコンテンツ領域横幅を取得する
    // この時点ではまだボタンを描画しないため
    // Popupのコンテンツ領域全体の横幅を取得できる
    // ChildWindowも幅0.0Fで残り幅いっぱいに作成しているため
    // この領域の右端とChildWindowの右端は一致する
    const float l_contentRegionWidth = ImGui::GetContentRegionAvail().x;

    // ホバー中は中身を薄く塗って枠を濃く、キーで選んでいるだけなら枠だけを濃くする
    // 見た目とEnterでの決定はEditorPopupNavigatorがまとめて行い、
    // クリックとEnterのどちらで押されてもtrueが返る
    auto& l_popupNavigator = a_deleteConfirmState.m_popupNavigator;

    l_result.m_isDeleteClicked = l_popupNavigator.Button(k_deleteLabel.data());

    // キャンセルボタン(削除ボタンと同じ行に描画)
    ImGui::SameLine();

    // ChildWindowの右端とキャンセルボタンの右端が完全に一致するようにX座標を指定する
    // コンテンツ領域右端         = ボタン行開始X + コンテンツ領域幅
    // キャンセルボタン左端       = コンテンツ領域右端 - キャンセルボタン幅
    // 従ってキャンセルボタン右端 = ChildWindow右端となる
    const float l_cancelButtonPositionX = l_buttonRowStartX + l_contentRegionWidth - a_cancelButtonWidth;

    ImGui::SetCursorPosX(l_cancelButtonPositionX);

    l_result.m_isCancelClicked = l_popupNavigator.Button(k_cancelLabel.data());

    return l_result;
}

void FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::HandleConfirm(const ButtonDrawResult& a_buttonDrawResult, AssetBrowserEditorWindow& a_editorWindow, Struct::AssetBrowserEditorWindowDeleteConfirmState& a_deleteConfirmState) const
{
    // クリック、またはキーで選んだボタンでのEnterで確定する
    // どちらもEditorPopupNavigator::Buttonの戻り値に入っている
    if (a_buttonDrawResult.m_isDeleteClicked)
    {
        // fileOperationはEditorWindowが所有しているため
        // コピーせず参照として取得する
        const auto& l_fileOperation = a_editorWindow.GetREFFileOperation();

        // 確認対象となっているファイル・フォルダを削除する
        l_fileOperation.Delete(a_deleteConfirmState.m_filePathList);

        // 削除確認Stateを初期状態へ戻す
        a_deleteConfirmState.m_filePathList.clear();
        a_deleteConfirmState.m_isActive = false;

        // 現在表示中のモーダルPopupを閉じる
        ImGui::CloseCurrentPopup();
    }
    else if (a_buttonDrawResult.m_isCancelClicked)
    {
        // キャンセルされたため
        // 削除対象として保持していたパス一覧を破棄する
        a_deleteConfirmState.m_filePathList.clear();
        a_deleteConfirmState.m_isActive = false;

        // 現在表示中のモーダルPopupを閉じる
        ImGui::CloseCurrentPopup();
    }
}

void FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::CollectFilePathRecursive(const std::filesystem::path& a_folderPath, std::vector<std::filesystem::path>& a_filePathList) const
{
    std::error_code l_errorCode = {};

    // directory_iteratorでa_folderPath直下のエントリを捜査
    // エラー時はdirectory_iteratorが空になるため安全
    for (const auto& l_entry : std::filesystem::directory_iterator(a_folderPath, l_errorCode))
    {
        // エントリのパスを追加
        a_filePathList.emplace_back(l_entry.path());

        // ディレクトリの場合は再帰的に中身を収集
        if (l_entry.is_directory())
        {
            CollectFilePathRecursive(l_entry.path(), a_filePathList);
        }
    }
}