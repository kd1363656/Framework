#include "EditorCloseConfirmPopup.h"

// ウィンドウの×ボタンが押されたときに、未保存の変更をどうするかを選んでもらう確認ダイアログ
// 保存して終了 / 保存せず終了 / キャンセル の3つのボタンを横に並べる
// 終了してよいと決まったらm_isCloseApprovedを立て、Application側がそれを見てループを抜ける
void FWK::Editor::EditorCloseConfirmPopup::Request()
{
    // OpenPopupは呼び出したときのIDスタックでポップアップのIDを作るため、
    // ここでは要求だけを覚えておき、Drawの中で開く
    m_isOpenRequested = true;
}

void FWK::Editor::EditorCloseConfirmPopup::Draw(EditorManager& a_editorManager)
{
    if (m_isOpenRequested)
    {
        ImGui::OpenPopup(k_titleLabel.data());

        m_isOpenRequested = false;
    }

    // ダイアログを画面の中央に出す
    // Pivot(0.5F, 0.5F)でダイアログ自身の中心がViewportの中心に来る
    if (const auto* l_mainViewport = ImGui::GetMainViewport();
        l_mainViewport)
    {
        ImGui::SetNextWindowPos(l_mainViewport->GetCenter(), ImGuiCond_Appearing, k_modalPopupPivot);
    }

    // モーダルなので表示中は後ろのエディターウィンドウを操作できない
    if (!ImGui::BeginPopupModal(k_titleLabel.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) { return; }

    // 左右キーとマウスでボタンを選べるようにする
    // 開いた直後は「保存して終了」がキーで選ばれた状態から始める
    m_popupNavigator.Begin(Enum::EditorPopupNavigationAxis::Horizontal, k_initialKeySelectedButtonIndex);

    DrawMessage();

    ImGui::Spacing();

    // 3つのボタンを同じ幅にして、ダイアログの描画可能領域の横幅ぴったりに並べる
    // 高さに0を渡すとImGuiが文字の高さに合わせた普通の高さにする
    const auto& l_buttonSize = ImVec2{ Utility::IMGUIFetchFitButtonWidth(k_buttonLabelList), Constant::k_imguiDefaultButtonHeight };

    // ボタンは描いた順番に0, 1, 2の番号が付き、左右キーで移動する
    const bool l_isSaveAndCloseClicked = m_popupNavigator.Button(k_saveAndCloseLabel.data(), l_buttonSize);

    ImGui::SameLine();

    const bool l_isCloseWithoutSaveClicked = m_popupNavigator.Button(k_closeWithoutSaveLabel.data(), l_buttonSize);

    ImGui::SameLine();

    const bool l_isCancelClicked = m_popupNavigator.Button(k_cancelLabel.data(), l_buttonSize);

    m_popupNavigator.End();

    if (l_isSaveAndCloseClicked)
    {
        // 保存してから終了する
        a_editorManager.SaveScene();

        m_isCloseApproved = true;

        ImGui::CloseCurrentPopup();
    }
    else if (l_isCloseWithoutSaveClicked)
    {
        // 変更を捨てて終了する
        m_isCloseApproved = true;

        ImGui::CloseCurrentPopup();
    }
    else if (l_isCancelClicked)
    {
        // 終了をやめてエディターに戻る
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void FWK::Editor::EditorCloseConfirmPopup::ApproveClose()
{
    // 確認が要らない場合(未保存の変更が無い、エディターを表示していない)はすぐに終了してよい
    m_isCloseApproved = true;
}

void FWK::Editor::EditorCloseConfirmPopup::DrawMessage() const
{
    const auto& l_sceneManager = SceneManager::GetInstance();

    // シーン名が空ならアウトライナーと同じ代わりの名前を出す
    auto l_sceneName = std::string{ k_emptySceneNameLabel };

    if (const auto& l_scene = l_sceneManager.GetVALScene().lock();
        l_scene &&
        !l_scene->GetREFName().empty())
    {
        l_sceneName = l_scene->GetREFName();
    }

    // 例 : シーン名が"Title"なら「シーン「Title」には保存されていない変更があります。」
    const auto& l_message = std::vformat(k_messageFormat, std::make_format_args(l_sceneName));

    ImGui::TextUnformatted(l_message.c_str());
}