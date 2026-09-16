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

    // 初回のみPopupを呼ぶ
    // 毎フレーム呼ぶとポップアップが感じても再オープンされてしまうため
    // フラグで初回のみ制御する
    if (l_deleteConfirmState.m_isOpenRequested)
    {
        // ImGui::OpenPopup : 指定IDのポップアップを次のBeginPopupModalで開く
        // 同じImGui::Begin / Endで呼ぶ必要がある
        ImGui::OpenPopup(k_titleLabel.data());

        l_deleteConfirmState.m_isOpenRequested = false;
    }

    // ポップアップ幅の事前計算
    // AlwaysAutoResize + GetContentRegionAvail().xの組み合わせは
    // 循環依存で(残り幅を図るために幅が必要)でウィンドウが伸び続ける
    // そのためBeginPopupModalの前に内容から必要な幅を計算し
    // SetNextWindowSizeConstraintsで幅を固定する
    // spacing計算にもGetContentRegionAvail()を使わず
    // この事前計算した幅から直接計算する
    // ボタン幅 = テキスト幅 + 左右パディング(FramePadding.x * 2)
    // ImGui::CalcTextSize : テキストの描画サイズを取得
    // ImGui::GetStyle().FramePadding.x : ボタンの左右パディング(片側部分)
    const float l_deleteButtonWidth = ImGui::CalcTextSize(k_deleteLabel.data()).x + ImGui::GetStyle().FramePadding.x * k_framePaddingBothSidesNUM;
    const float l_cancelButtonWidth = ImGui::CalcTextSize(k_cancelLabel.data()).x + ImGui::GetStyle().FramePadding.x * k_framePaddingBothSidesNUM;

    // 内容幅 = メッセージテキスト幅とボタン行幅の大きい方
    // ImGui::GetStyle().ItemInnerSpacing.x : 同一行アイテム間の最少スペース
    const float l_messageWidth   = ImGui::CalcTextSize(k_messageLabel.data()).x;
    const float l_buttonRowWidth = l_deleteButtonWidth + l_cancelButtonWidth + ImGui::GetStyle().ItemInnerSpacing.x;
    const float l_contentWidth   = std::max(l_messageWidth, l_buttonRowWidth);

    // ポップアップ幅 = 内容幅 + ウィンドウ左右パディング
    const float l_popupWidth = l_contentWidth + ImGui::GetStyle().WindowPadding.x * k_framePaddingBothSidesNUM;

    // 幅を固定(高さは0.0F ~ FLT_MAXで自動調整)
    // ImGui::SetNextWindowSizeConstraints(最小サイズ、最大サイズ)
    // 最小幅 = 最大幅 = l_popupWidthで幅を固定し
    // 高さは0 ~ FLT_MAXでAlwaysAutoResizeが高さのみ自動調整
    ImGui::SetNextWindowSizeConstraints( ImVec2{ l_popupWidth, Constant::k_imguiRemainingSize.y }, ImVec2{ l_popupWidth, FLT_MAX } );

    // モーダルポップアップを開く
    // ImGui::BeginPopupModal(ラベル、
    //                        開閉状態フラグ(nullptr = 自動管理)、
    //                        ウィンドウフラグ);
    // ImGuiWindowFlags_AlwaysAutoResize : 内容に合わせてサイズ自動調整
    // ※幅はSetNextWindowSizeConstraintsで固定済みのため高さのみ自動調整される
    // モーダルは背後のウィンドウ操作をブロックする
    // 戻り値false = ポップアップが閉じている
    if (!ImGui::BeginPopupModal(k_titleLabel.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // ポップアップが閉じた場合は非アクティブにする
        l_deleteConfirmState.m_isActive = false;

        return;
    }

    // メッセージ描画
    // ImGui::TextUnformatted : フォーマットなしテキスト描画
    ImGui::TextUnformatted(k_messageLabel.data());
    ImGui::Spacing        ();

    // 削除対象ファイルパスリスト描画
    // スクロール可能なChildWindowで描画
    // ImGui::BeginChild(ラベル、
    //                   領域サイズ(幅0 = 残り幅いっぱい、高さ = 固定)、
    //                   ボーダー描画有無);
    // 幅 = 0ポップアップ幅(固定)からパディングを引いた分が割り当てられる
    // ChildWindow内でY軸スクロールが可能
    if (ImGui::BeginChild(k_childLabel.data(), ImVec2{ Constant::k_imguiRemainingSize.x, k_fileListChildHeight }, true))
    {
        // 削除対象のファイルパスを上から順に描画
        for (const auto& l_filePath : l_deleteConfirmState.m_filePathList)
        {
            ImGui::TextUnformatted(l_filePath.generic_string().c_str());
        }
    }

    ImGui::EndChild();
    ImGui::Spacing ();

    // 削除ボタン
    // ImGui::Button(ラベル、サイズ) : ボタンを描画
    // サイズ(0, 0) = 内容に合わせて自動サイズ
    if (ImGui::Button(k_deleteLabel.data()))
    {
        // 削除実行
        auto l_fileOperation = a_editorWindow.GetMutableREFFileOperation();

        l_fileOperation.Delete(l_deleteConfirmState.m_filePathList);

        // Stateをクリア
        l_deleteConfirmState.m_filePathList.clear();
        l_deleteConfirmState.m_isActive = false;

        // モーダルを閉じる
        // ImGui::CloseCurrentPopup : 現在のポップアップを閉じる
        ImGui::CloseCurrentPopup();
    }

    // キャンセルの端(右端)
    // キャンセルボタンの右端がウィンドウ右端と一致するように配置
    const float l_spacing = std::max(Constant::k_imguiRemainingSize.x, l_contentWidth - l_deleteButtonWidth - l_cancelButtonWidth);

    // ImGui::SameLine(開始位置、スペース)
    // spacing方式によりImGuiがカーソル位置を追跡し
    // ポップアップ幅にキャンセルボタンが膨れる -> クリップされない
    ImGui::SameLine(Constant::k_imguiRemainingSize.x, l_spacing);

    if (ImGui::Button(k_cancelLabel.data()))
    {
        // Stateをクリア
        l_deleteConfirmState.m_filePathList.clear();
        l_deleteConfirmState.m_isActive = false;

        // モーダルを閉じる
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}