#include "EditorPopupNavigator.h"

// ポップアップの中の項目(メニュー項目・ボタン)を、マウスとキーボードの両方で操作するためのクラス
// マウスが乗っている項目   : 中身を薄く塗り、枠を濃く塗る(文字の色は変えない)
// キーで選んでいる項目     : 枠だけを濃く塗る
// マウスとキーの選択は連動しないため、マウスを動かしてもキーで選んだ位置は変わらない
// 使い方 : BeginPopupなどが成功した直後にBegin、項目をMenuItem / Buttonで描き、EndPopupの前にEnd
void FWK::Editor::EditorPopupNavigator::Begin(const Enum::EditorPopupNavigationAxis a_axis, const std::size_t& a_initialKeySelectedIndex)
{
    // ポップアップが開いた最初のフレームは、キーの選択位置を初期位置へ戻す
    // 例 : 確認ダイアログを開くたびに「保存して終了」が選ばれた状態から始める
    if (ImGui::IsWindowAppearing())
    {
        m_keySelectedIndex = a_initialKeySelectedIndex;

        m_itemEnabledList.clear();
    }

    m_drawingItemEnabledList.clear();

    m_isConfirmRequested = false;

    // マウスが乗った項目の中身を明るい灰色で塗る
    // ポップアップの背景(#383838)は一覧の背景より明るいので、一覧と同じホバー色(#383838)だと見えなくなる
    // そのためUEのメニューと同じく一段明るいホバー色(#575757)を使う
    // MenuItemはHeader系、ButtonはButton系の色で背景を塗るため両方を差し替える
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Constant::k_imguiRaisedItemHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,  Constant::k_imguiItemActiveColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Constant::k_imguiRaisedItemHoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Constant::k_imguiItemActiveColor);

    // このポップアップにフォーカスがあるときだけキーを受け付ける
    // サブメニューと親メニューが同時に開いているときに、両方が同じキーで動かないようにするため
    if (!ImGui::IsWindowFocused()) { return; }

    // メニューは上下、横に並んだボタンは左右のキーで選択を動かす
    const bool l_isVertical = a_axis == Enum::EditorPopupNavigationAxis::Vertical;
    const auto l_prevKey    = l_isVertical ? ImGuiKey_UpArrow   : ImGuiKey_LeftArrow;

    if (const auto l_nextKey = l_isVertical ? ImGuiKey_DownArrow : ImGuiKey_RightArrow;
        ImGui::IsKeyPressed(l_nextKey))
    {
        MoveKeySelection(true);
    }
    else if (ImGui::IsKeyPressed(l_prevKey))
    {
        MoveKeySelection(false);
    }

    // Enterでキーで選んでいる項目を決定する
    m_isConfirmRequested = ImGui::IsKeyPressed(ImGuiKey_Enter) ||
                           ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);
}
void FWK::Editor::EditorPopupNavigator::End()
{
    ImGui::PopStyleColor(k_pushStyleColorNUM);

    // このフレームで描いた項目の有効/無効を、次のフレームのキー操作で使う
    // swapなのでメモリを確保し直さずに入れ替えられる
    m_itemEnabledList.swap(m_drawingItemEnabledList);
}

bool FWK::Editor::EditorPopupNavigator::MenuItem(const char* a_label, const char* a_shortcut, const bool a_isEnabled)
{
    const bool l_isClicked = ImGui::MenuItem(a_label,
                                             a_shortcut,
                                             false,
                                             a_isEnabled);

    return ApplyItemState(l_isClicked, a_isEnabled);
}
bool FWK::Editor::EditorPopupNavigator::Button(const char* a_label, const ImVec2& a_size)
{
    const bool l_isClicked = ImGui::Button(a_label, a_size);

    return ApplyItemState(l_isClicked, true);
}

bool FWK::Editor::EditorPopupNavigator::ApplyItemState(const bool a_isClicked, const bool a_isEnabled)
{
    // 描いた順番がそのまま項目の番号になる
    // 例 : コピー(0)、切り取り(1)、貼り付け(2) の順で描けば、↓を2回押すと貼り付けが選ばれる
    const auto& l_itemIndex = m_drawingItemEnabledList.size();

    m_drawingItemEnabledList.emplace_back(a_isEnabled);

    const bool l_isKeySelected = l_itemIndex == m_keySelectedIndex;

    // 中身の薄い塗りはImGuiがマウスの乗った項目にだけ行うため、ここでは枠だけを描く
    // キーで選んでいるだけの項目は塗りが付かず、枠だけになる
    if ((a_isEnabled             &&
         ImGui::IsItemHovered()) ||
        l_isKeySelected)
    {
        DrawItemFrame();
    }

    // クリックされたか、キーで選んでいる項目でEnterが押されたら決定
    return a_isClicked      ||
           (l_isKeySelected &&
            a_isEnabled     &&
            m_isConfirmRequested);
}

void FWK::Editor::EditorPopupNavigator::MoveKeySelection(const bool a_isNext)
{
    // 前のフレームで描いた項目の数
    const auto& l_itemCount = m_itemEnabledList.size();

    if (l_itemCount == k_emptyItemCount) { return; }

    // 1つずつ進め、有効な項目で止まる(無効な項目は飛ばす)
    // 最後まで行ったら最初へ戻る
    // 例 : 項目[有効, 無効, 有効]で0番を選択中に↓ → 1番は無効なので2番へ
    auto l_index = m_keySelectedIndex;

    for (std::size_t l_step = 0ULL; l_step < l_itemCount; ++l_step)
    {
        // まだ何も選んでいなければ、↓なら先頭、↑なら末尾から始める
        if (l_index >= l_itemCount)
        {
            l_index = a_isNext ? k_firstItemIndex : l_itemCount - k_keySelectionStep;
        }
        else
        {
            l_index = a_isNext ? (l_index + k_keySelectionStep) % l_itemCount : (l_index + l_itemCount - k_keySelectionStep) % l_itemCount;
        }

        if (!m_itemEnabledList[l_index]) { continue; }

        m_keySelectedIndex = l_index;

        return;
    }
}

void FWK::Editor::EditorPopupNavigator::DrawItemFrame() const
{
    // 直前に描いた項目の矩形に沿って濃い色の枠を描く
    auto* l_drawList = ImGui::GetWindowDrawList();

    if (!l_drawList) { return; }

    const auto& l_style   = ImGui::GetStyle      ();
    const auto& l_itemMIN = ImGui::GetItemRectMin();
    const auto& l_itemMAX = ImGui::GetItemRectMax();

    l_drawList->AddRect(l_itemMIN,
                        l_itemMAX,
                        ImGui::GetColorU32(Constant::k_imguiItemSelectedColor),
                        l_style.FrameRounding,
                        ImDrawFlags_None,
                        k_itemFrameThickness);
}