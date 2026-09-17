#include "AssetBrowserEditorWindowDeleteConfirmPopup.h"

void FWK::Editor::AssetBrowserEditorWindowDeleteConfirmPopup::Request(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindow& a_editorWindow) const
{
    auto& l_deleteConfirmState = a_editorWindow.GetMutableREFDeleteConfirmState();

    // 削除対象のファイルパスリストをコピー
    l_deleteConfirmState.m_filePathList = a_filePathList;

    // 確認ダイアログをアクティブにする
    l_deleteConfirmState.m_isActive = true;

    // OpenPopup要求フラグを立てる
    // Requestは別ポップアップ内から呼ばれる可能性があるため
    // ここではOpenPopupを呼ばずDraw側で呼ぶ
    l_deleteConfirmState.m_isOpenRequested = Struct::AssetBrowserEditorWindowDeleteConfirmState::k_initialIsOpenRequested;
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
    const float l_cancelButtonWidth = ImGui::CalcTextSize(k_cancelLabel.data()).x + ImGui::GetStyle().FramePadding.x * k_framePaddingBothSidesNUM;

    const auto* l_mainViewport = ImGui::GetMainViewport();

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

    // 削除確認メッセージを表示
    ImGui::TextUnformatted(k_messageLabel.data());
    ImGui::Spacing        ();

    // 削除対象の一覧をスクロール可能なChildWindowとして表示する
    // 仮に0.0Fを指定すると
    // 親Windowのコンテンツ領域の残り幅いっぱいまで使用する
    if (const ImVec2& l_childWindowSize = { Constant::k_imguiRemainingSize.x, k_fileListChildHeight };
        ImGui::BeginChild(k_childLabel.data(), l_childWindowSize , true))
    {
        for (const auto& l_filePath : l_deleteConfirmState.m_filePathList)
        {
            // generic_stringを使用して
            ImGui::TextUnformatted(l_filePath.generic_string().c_str());
        }
    }

    ImGui::EndChild();
    ImGui::Spacing ();

    // ボタン行を描画し始める時点のX座標を保持する
    // この位置はPopupのコンテンツ領域左端になる
    // 後でキャンセルボタンを右端へ配置する際の基準として使用する
    const float l_buttonRowStartX = ImGui::GetCursorPosX();

    // 現在位置から使用可能なコンテンツ領域の横幅を取得する
    // この時点ではまだボタンを描画しないため
    // Popupのコンテンツ領域全体の横幅を取得できる
    // ChildWindowも幅0.0Fで残り幅いっぱいに作成しているため
    // この領域に右端とChildWindowの右端は一致する
    const float l_contentRegionWidth = ImGui::GetContentRegionAvail().x;

    // 削除ボタン
    if (ImGui::Button(k_deleteLabel.data()))
    {
        // FileOperationはEditorWindowが所有しているため
        // コピーせず参照として取得する
        auto& l_fileOperation = a_editorWindow.GetMutableREFFileOperation();

        // 確認対象となっているファイル・フォルダを削除する
        l_fileOperation.Delete(l_deleteConfirmState.m_filePathList);

        // 削除確認Stateを初期状態へ戻す
        l_deleteConfirmState.m_filePathList.clear();
        l_deleteConfirmState.m_isActive = false;

        // 現在表示中のモーダルPopupを閉じる
        ImGui::CloseCurrentPopup();
    }

    // キャンセルボタン(削除ボタンと同じ行に描画)
    ImGui::SameLine();

    // ChildWindowの右端とキャンセルボタンの右端が完全に一致するよにX表を直接指定する
    // コンテンツ領域右端         = ボタン行開始X + コンテンツ領域幅
    // キャンセルボタン左端       = コンテンツ領域右端 - キャンセルボタン幅し
    // 従ってキャンセルボタン右端 = ChildWindow右端となる
    const float l_cancelButtonPositionX = l_buttonRowStartX + l_contentRegionWidth - l_cancelButtonWidth;

    ImGui::SetCursorPosX(l_cancelButtonPositionX);

    if (ImGui::Button(k_cancelLabel.data()))
    {
        // キャンセルされたため、
        // 削除対象として保持していたパス一覧を破棄する
        l_deleteConfirmState.m_filePathList.clear();
        l_deleteConfirmState.m_isActive = false;

        // 現在表示中のモーダルPopupを閉じる
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}