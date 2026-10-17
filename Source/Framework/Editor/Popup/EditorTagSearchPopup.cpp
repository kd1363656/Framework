#include "EditorTagSearchPopup.h"

// Unityの「Add Component」と同じ形の、タグ(カテゴリー)と検索を持つポップアップ
// 何を追加するのか(コンポーネント・ノードなど)は知らず、TaggedFactoryのタグと型の名前の一覧だけを受け取り、決定した型の名前を返す
// 例 : TaggedFactory<GameObjectComponentBase>の一覧 → "カメラ" ▶ "GameObjectCameraComponent"
// 使い方 : ImGui::OpenPopupで開き、毎フレームDrawを呼ぶ。決定したフレームだけ空でない名前が返る
// ※注意 : タグと項目の名前は、文字列リテラル(終端の'\0'がある静的な文字列)を指すstd::string_viewにする
//          ImGuiへdata()をそのまま渡すのと、選んだタグの名前をフレームをまたいで覚えておくため
std::string_view FWK::Editor::EditorTagSearchPopup::Draw(const std::vector<Struct::TaggedFactoryTagGroup>& a_tagGroupList,
                                                         const std::string_view&                           a_popupLabel,
                                                         const ImVec2&                                     a_popupPosition,
                                                         const Struct::EditorTagSearchPopupText&           a_popupText,
                                                         const float                                       a_popupWidth)
{
    // 呼び出し側が決めた位置(例 : ボタンの真下)に、決めた幅で開く
    // 高さは0.0Fを渡すと中身に合わせて自動で決まる
    ImGui::SetNextWindowPos (a_popupPosition);
    ImGui::SetNextWindowSize(ImVec2{ a_popupWidth, k_autoFitPopupHeight });

    if (!ImGui::BeginPopup(a_popupLabel.data())) { return {}; }

    // ImGui::IsWindowAppearing : ポップアップが開いた最初のフレームだけtrueを返す
    // 前回開いたときの検索文字列や選んでいたタグが残らないよう、開くたびに初期状態へ戻す
    if (ImGui::IsWindowAppearing())
    {
        Reset();
    }

    // 検索欄の入力で検索中かどうかが変わる前の状態を覚えておく
    // 例 : Backspaceで最後の1文字を消したフレームに、タグの一覧へ戻る処理まで動いてしまわないようにするため
    const bool l_wasSearching = IsSearching();

    DrawSearchBar();
    DrawHeader   (a_popupText);

    ImGui::Separator();

    // 今の状態(検索中 / タグを選ぶ前 / タグを選んだ後)に合わせて、一覧に並べる名前を集める
    // 一覧の形はショートカットで状態が変わる前に決めておき、このフレームの描画で使う
    const auto& l_itemList       = FetchVALItemList(a_tagGroupList);
    const bool  l_isTagList = IsTagList  ();

    // 検索文字列が変わって一覧が短くなった場合に、ハイライトの位置が一覧の外を指さないようにする
    if (!l_itemList.empty() &&
        m_highlightIndex >= l_itemList.size())
    {
        m_highlightIndex = l_itemList.size() - k_highlightStep;
    }

    const auto& l_shortcutDecidedItem = HandleShortcut(l_itemList, l_wasSearching);
    const auto& l_clickDecidedItem    = DrawItemList  (l_itemList, a_popupText, l_isTagList);

    ImGui::EndPopup();

    return l_shortcutDecidedItem.empty() ? l_clickDecidedItem : l_shortcutDecidedItem;
}

void FWK::Editor::EditorTagSearchPopup::Reset()
{
    // 検索文字列・選んでいるタグ・ハイライトの位置を初期状態に戻す
    m_searchBuffer.fill(Constant::k_nullCharacter);

    m_selectedTag    = {};
    m_highlightIndex = k_initialHighlightIndex;

    // 開いてすぐに文字を入力できるよう、次の描画で検索欄へフォーカスする
    m_shouldFocusSearchBar    = true;
    m_shouldScrollToHighlight = false;
}

void FWK::Editor::EditorTagSearchPopup::DrawSearchBar()
{
    // 行の先頭に虫眼鏡のアイコンを描画し、同じ行の右側に検索欄を置く
    DrawSearchIcon();

    ImGui::SameLine();

    const bool  l_isSearching    = IsSearching                 ();
          float l_inputTextWidth = ImGui::GetContentRegionAvail().x;

    // 1文字でも入力されているときは、検索欄の右に×ボタンを置くため、その分だけ検索欄を短くする
    // ×ボタンの幅 = アイコンの幅 + 左右の余白(FramePadding) + 検索欄との間隔(ItemSpacing)
    // CalcTextSizeの第3引数をtrueにすると、ラベルの"##"より後ろ(ImGuiのID)を幅に含めない
    if (l_isSearching)
    {
        const auto& l_style            = ImGui::GetStyle    ();
        const float l_clearButtonWidth = ImGui::CalcTextSize(k_clearSearchButtonLabel.data(), nullptr, true).x + l_style.FramePadding.x * Constant::k_imguiFramePaddingBothSidesNUM + l_style.ItemSpacing.x;

        l_inputTextWidth -= l_clearButtonWidth;
    }

    // ImGui::SetKeyboardFocusHere : 次に描画する入力欄へキーボードのフォーカスを移す
    if (m_shouldFocusSearchBar)
    {
        ImGui::SetKeyboardFocusHere();

        m_shouldFocusSearchBar = false;
    }

    ImGui::SetNextItemWidth(l_inputTextWidth);

    // InputTextWithHint(ImGuiのID、
    //                   何も入力されていないときに薄く表示する文字列、
    //                   入力内容を書き込む配列、
    //                   配列の大きさ);
    // 入力内容が変わったフレームだけtrueを返す
    if (ImGui::InputTextWithHint(k_searchInputTextLabel.data(),
                                 k_searchHintText.data(),
                                 m_searchBuffer.data(),
                                 m_searchBuffer.size()))
    {
        // 一覧の中身が変わるため、ハイライトを先頭へ戻す
        m_highlightIndex = k_initialHighlightIndex;
    }

    if (!l_isSearching) { return; }

    ImGui::SameLine();

    // クリックしたら入力内容をすべて消し、続けて入力できるよう検索欄へフォーカスを戻す
    if (ImGui::Button(k_clearSearchButtonLabel.data()))
    {
        m_searchBuffer.fill(Constant::k_nullCharacter);

        m_highlightIndex       = k_initialHighlightIndex;
        m_shouldFocusSearchBar = true;
    }
}
void FWK::Editor::EditorTagSearchPopup::DrawHeader(const Struct::EditorTagSearchPopupText& a_popupText)
{
    // 検索中はタグに関係なく、すべてのタグから一致した項目を並べる
    // そのため戻る矢印は出さず、「検索結果」だけを表示する
    if (IsSearching())
    {
        DrawCenteredText(k_searchResultText);

        return;
    }

    // まだタグを選んでいなければ、呼び出し側が決めた見出し(例 : 「コンポーネントの種類」)を表示する
    if (m_selectedTag.empty())
    {
        DrawCenteredText(a_popupText.m_tagListHeaderText);

        return;
    }

    // タグを選んだ後は、左端の三角形(<)と選んだタグの名前を含めた、行全体を1つの当たり判定にする
    // 幅を指定しない(0.0F)と行の右端まで広がるため、名前の右側を押しても反応する
    // ImGuiSelectableFlags_NoAutoClosePopups : ポップアップ直下のSelectableはクリックでポップアップを閉じるため、閉じないようにする
    if (ImGui::Selectable(k_backHeaderLabel.data(), false, ImGuiSelectableFlags_NoAutoClosePopups))
    {
        ClearSelectedTag();
    }

    // 直前の項目(Selectable)の左端に三角形を描く
    DrawItemEdgeIcon(k_imguiFontAwesomeCaretLeftIcon, false);

    // 同じ行に、選んだタグの名前をポップアップの真ん中へ表示する
    // 名前は項目ではなく文字なので、Selectableのクリックを邪魔しない
    ImGui::SameLine();

    DrawCenteredText(m_selectedTag);
}

std::string_view FWK::Editor::EditorTagSearchPopup::DrawItemList(const std::vector<std::string_view>&    a_itemList,
                                                                 const Struct::EditorTagSearchPopupText& a_popupText,
                                                                 const bool                              a_isTagList)
{
    // 一覧は高さを固定した子ウィンドウに並べ、項目が多いときはスクロールできるようにする
    // ImGui::BeginChildはfalseを返した場合もEndChildを呼ぶ必要がある
    if (!ImGui::BeginChild(k_itemListChildLabel.data(), k_itemListChildSize))
    {
        ImGui::EndChild();

        return {};
    }

    if (a_itemList.empty())
    {
        ImGui::TextDisabled(a_popupText.m_noItemText.data());

        ImGui::EndChild();

        return {};
    }

    std::string_view l_decidedItem = {};

    for (std::size_t l_itemIndex = 0ULL; l_itemIndex < a_itemList.size(); ++l_itemIndex)
    {
        const auto& l_item          = a_itemList[l_itemIndex];
        const bool  l_isHighlighted = l_itemIndex == m_highlightIndex;

        // ImGuiのIDは文字列から作られるため、添字をIDに混ぜて同じ名前が並んでも重ならないようにする
        // ※注意 : 項目のアドレスをIDにしてはいけない
        //          一覧はフレームごとに作り直すため、アドレスが変わるとIDも変わる
        //          クリックはマウスを押したフレームと離したフレームで同じIDである必要があり、IDが変わると反応しなくなる
        ImGui::PushID(static_cast<int>(l_itemIndex));

        // タグを選んだときは、ポップアップを開いたまま中の項目の一覧へ進むため、クリックで閉じない
        const bool l_isClicked = ImGui::Selectable(l_item.data(), l_isHighlighted, ImGuiSelectableFlags_NoAutoClosePopups);

        ImGui::PopID();

        // タグの行には、中へ進めることを表す">"を右端に描く
        // GetItemRectMin / GetItemRectMaxが直前の項目を指すため、Selectableの直後に呼ぶ
        if (a_isTagList)
        {
            DrawItemEdgeIcon(k_imguiFontAwesomeCaretRightIcon, true);
        }

        // キー操作でハイライトが動いたときは、ハイライト中の項目が見える位置までスクロールする
        if (l_isHighlighted &&
            m_shouldScrollToHighlight)
        {
            ImGui::SetScrollHereY();

            m_shouldScrollToHighlight = false;
        }

        if (!l_isClicked) { continue; }

        // マウスで選んだ項目もハイライトし、キー操作と同じ処理で決定する
        m_highlightIndex = l_itemIndex;
        l_decidedItem    = DecideItem(l_item);

        // 決定した時点で一覧の中身が変わるため、このフレームの描画はここで終える
        break;
    }

    ImGui::EndChild();

    return l_decidedItem;
}

void FWK::Editor::EditorTagSearchPopup::DrawSearchIcon() const
{
    // 虫眼鏡のアイコンは、通常の文字の大きさだと検索欄に対して小さく見えるため、少し大きく描く
    // 文字として並べると大きくした分だけ行の高さと位置がずれるため、
    // 描画リストで検索欄(フレーム)の高さの真ん中に描き、アイコンの幅と検索欄の高さの空きだけをレイアウトに確保する
    auto* l_font     = ImGui::GetFont          ();
    auto* l_drawList = ImGui::GetWindowDrawList();

    if (!l_font     ||
        !l_drawList)
    {
        return;
    }

    const float l_iconFontSize = ImGui::GetFontSize   () * k_searchIconScale;
    const float l_frameHeight  = ImGui::GetFrameHeight();

    // CalcTextSizeA(文字の大きさ、
    //               折り返さない最大幅、
    //               折り返し幅(0.0Fで折り返さない)、
    //               文字列の先頭、
    //               文字列の末尾);
    const auto& l_iconSize = l_font->CalcTextSizeA(l_iconFontSize,
                                                   k_unlimitedTextWidth,
                                                   k_noWrapWidth,
                                                   k_imguiFontAwesomeMagnifyingGlassIcon.data(),
                                                   k_imguiFontAwesomeMagnifyingGlassIcon.data() + k_imguiFontAwesomeMagnifyingGlassIcon.size());

    const auto& l_cursorPosition = ImGui::GetCursorScreenPos();
    const auto& l_iconPosition   = ImVec2{ l_cursorPosition.x, l_cursorPosition.y + (l_frameHeight - l_iconSize.y) * Constant::k_halfMagnification };

    // AddText(文字のフォント、
    //         文字の大きさ、
    //         描く位置、
    //         文字の色、
    //         文字列の先頭、
    //         文字列の末尾);
    l_drawList->AddText(l_font,
                        l_iconFontSize,
                        l_iconPosition,
                        ImGui::GetColorU32(ImGuiCol_Text),
                        k_imguiFontAwesomeMagnifyingGlassIcon.data(),
                        k_imguiFontAwesomeMagnifyingGlassIcon.data() + k_imguiFontAwesomeMagnifyingGlassIcon.size());

    // 描画リストへ直接描いた分のレイアウトを確保する
    // Dummyはクリックを受け取らないため、アイコンの上のクリックを邪魔しない
    ImGui::Dummy(ImVec2{ l_iconSize.x, l_frameHeight });
}
void FWK::Editor::EditorTagSearchPopup::DrawItemEdgeIcon(const std::string_view& a_icon, const bool a_isRightEdge) const
{
    // 直前に描画した項目(Selectable)の端に、アイコン(">"や"<")を項目の名前とは独立した位置に描く
    // 右端 : 中へ進めることを表す">"  左端 : 一覧へ戻ることを表す"<"
    auto* l_drawList = ImGui::GetWindowDrawList();

    if (!l_drawList) { return; }

    const auto& l_style    = ImGui::GetStyle      ();
    const auto& l_itemMIN  = ImGui::GetItemRectMin();
    const auto& l_itemMAX  = ImGui::GetItemRectMax();
    const auto& l_iconSize = ImGui::CalcTextSize  (a_icon.data(), a_icon.data() + a_icon.size());

    // 横 : 右端なら項目の右端からアイコンの幅と余白(FramePadding)だけ内側、左端なら項目の左端から余白だけ内側
    // 縦 : 項目の高さの真ん中
    // 例 : 項目が x = 100〜300、アイコンの幅が 8、FramePadding.x が 4 → 右端は x = 300 - 8 - 4 = 288、左端は x = 100 + 4 = 104
    const float l_iconPositionX = a_isRightEdge ? l_itemMAX.x - l_iconSize.x - l_style.FramePadding.x : l_itemMIN.x + l_style.FramePadding.x;
    const float l_iconPositionY = l_itemMIN.y + (l_itemMAX.y - l_itemMIN.y - l_iconSize.y) * Constant::k_halfMagnification;

    l_drawList->AddText(ImVec2{ l_iconPositionX, l_iconPositionY }, ImGui::GetColorU32(ImGuiCol_TextDisabled), a_icon.data(), a_icon.data() + a_icon.size());
}
void FWK::Editor::EditorTagSearchPopup::DrawCenteredText(const std::string_view& a_text) const
{
    // ポップアップの幅の真ん中に文字列を置く
    // SameLineで矢印の後ろに描画した場合も、矢印の位置とは関係なく真ん中に置く
    const float l_textWidth   = ImGui::CalcTextSize  (a_text.data(), a_text.data() + a_text.size()).x;
    const float l_windowWidth = ImGui::GetWindowWidth();

    ImGui::SetCursorPosX  ((l_windowWidth - l_textWidth) * Constant::k_halfMagnification);
    ImGui::TextUnformatted(a_text.data(), a_text.data() + a_text.size());
}

std::string_view FWK::Editor::EditorTagSearchPopup::HandleShortcut(const std::vector<std::string_view>& a_itemList, const bool a_wasSearching)
{
    // ←とBackspaceでタグの一覧へ戻る
    // 検索欄に文字が入っているときは、カーソルの移動や文字の削除に使うため戻らない
    if ((ImGui::IsKeyPressed(ImGuiKey_LeftArrow)  ||
         ImGui::IsKeyPressed(ImGuiKey_Backspace)) &&
        !a_wasSearching                           &&
        !m_selectedTag.empty())
    {
        ClearSelectedTag();

        return {};
    }

    if (a_itemList.empty()) { return {}; }

    // ↑と↓でハイライトを1つずつ動かす(一覧の端では止める)
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) &&
        m_highlightIndex > k_initialHighlightIndex)
    {
        --m_highlightIndex;

        m_shouldScrollToHighlight = true;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) &&
        m_highlightIndex + k_highlightStep < a_itemList.size())
    {
        ++m_highlightIndex;

        m_shouldScrollToHighlight = true;
    }

    // Enterでハイライト中の項目を決定する
    // 押し続けたときに2回決定しないよう、キーリピートは使わない
    const bool l_isEnterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter,       false) ||
                                  ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);

    // →はタグの一覧で、ハイライト中のタグへ進むときだけ使う
    const bool l_isEnterTagPressed = !a_wasSearching        &&
                                     m_selectedTag.empty() &&
                                     ImGui::IsKeyPressed(ImGuiKey_RightArrow, false);

    if (!l_isEnterPressed         &&
        !l_isEnterTagPressed)
    {
        return {};
    }

    return DecideItem(a_itemList[m_highlightIndex]);
}

std::string_view FWK::Editor::EditorTagSearchPopup::DecideItem(const std::string_view& a_item)
{
    // タグの一覧で決定したときは、そのタグの項目の一覧へ進む(まだ何も決まっていないので空を返す)
    if (IsTagList())
    {
        m_selectedTag    = a_item;
        m_highlightIndex = k_initialHighlightIndex;

        // 検索欄でEnterを押すと検索欄のフォーカスが外れるため、続けて入力できるよう戻す
        m_shouldFocusSearchBar = true;

        return {};
    }

    // 項目の一覧(検索結果を含む)で決定したときは、ポップアップを閉じて決定した項目の名前を返す
    ImGui::CloseCurrentPopup();

    return a_item;
}

void FWK::Editor::EditorTagSearchPopup::ClearSelectedTag()
{
    // 選んでいたタグを消して、タグの一覧の状態へ戻す
    m_selectedTag          = {};
    m_highlightIndex       = k_initialHighlightIndex;
    m_shouldFocusSearchBar = true;
}

bool FWK::Editor::EditorTagSearchPopup::IsSearching() const
{
    // 配列の先頭が終端文字でなければ、1文字以上入力されている
    return m_searchBuffer.front() != Constant::k_nullCharacter;
}
bool FWK::Editor::EditorTagSearchPopup::IsTagList() const
{
    // 検索中でなく、まだタグを選んでいなければタグの一覧
    return !IsSearching()        &&
           m_selectedTag.empty();
}
bool FWK::Editor::EditorTagSearchPopup::IsMatchedSearchText(const std::string_view& a_text) const
{
    const auto& l_searchText = std::string_view{ m_searchBuffer.data() };

    // 名前の中に検索文字列が含まれているかを、大文字と小文字を区別せずに調べる
    // std::ranges::search : 見つかった範囲を返し、見つからなければ空の範囲を返す
    // 例 : "camera"で検索すると"GameObjectCameraComponent"が一致する
    const auto& l_matchedRange = std::ranges::search(a_text,
                                                     l_searchText,
                                                     [](const char a_left, const char a_right)
                                                     {
                                                         return std::tolower(static_cast<unsigned char>(a_left)) == std::tolower(static_cast<unsigned char>(a_right));
                                                     });

    return !l_matchedRange.empty();
}

std::vector<std::string_view> FWK::Editor::EditorTagSearchPopup::FetchVALItemList(const std::vector<Struct::TaggedFactoryTagGroup>& a_tagGroupList) const
{
    std::vector<std::string_view> l_itemList = {};

    // 検索中は、すべてのタグの項目から名前が一致したものを集める
    if (IsSearching())
    {
        for (const auto& l_tagGroup : a_tagGroupList)
        {
            for (const auto& l_itemName : l_tagGroup.m_typeNameList)
            {
                if (!IsMatchedSearchText(l_itemName)) { continue; }

                l_itemList.emplace_back(l_itemName);
            }
        }

        return l_itemList;
    }

    // タグを選ぶ前は、項目を1つ以上持つタグの名前を集める
    // 中が空のタグへ進んでも何もできないため並べない
    if (m_selectedTag.empty())
    {
        l_itemList.reserve(a_tagGroupList.size());

        for (const auto& l_tagGroup : a_tagGroupList)
        {
            if (l_tagGroup.m_typeNameList.empty()) { continue; }

            l_itemList.emplace_back(l_tagGroup.m_tag);
        }

        return l_itemList;
    }

    // タグを選んだ後は、そのタグの項目を集める
    const auto& l_selectedTagGroupITR = std::ranges::find(a_tagGroupList, m_selectedTag, &Struct::TaggedFactoryTagGroup::m_tag);

    if (l_selectedTagGroupITR == a_tagGroupList.end()) { return l_itemList; }

    return l_selectedTagGroupITR->m_typeNameList;
}