#include "EditorAddMenu.h"

void FWK::Editor::EditorAddMenu::Open()
{
    // ノードエディターなどボタンを置かない場所用
    // 右クリックされたマウスの位置にメニューの左上を合わせて開く
    // ボタンが無いので幅は既定の値を使う
    OpenAt(ImGui::GetMousePos(), k_mousePopupWidth);
}

const FWK::TypeINFORegistry::TypeINFO* FWK::Editor::EditorAddMenu::DrawButton(const std::string_view& a_buttonLabel, const std::string_view& a_rootLabel, const std::vector<std::string_view>& a_categoryNameList)
{
    // UnityのAdd Componentと同じく、文字の長さに関係なく決まった幅の大きいボタンにする
    // ウィンドウがそれより狭い時はウィンドウの幅に縮める
    const float l_buttonWidth = std::min(k_buttonWidth, ImGui::GetContentRegionAvail().x);

    // 呼び出したウィンドウの残りの横幅の真ん中にボタンが来るようにする
    ShiftCursorToCenter(l_buttonWidth);

    // 高さに0を渡すと、高さだけは文字に合わせた普通のボタンの高さになる
    if (ImGui::Button(a_buttonLabel.data(), ImVec2(l_buttonWidth, k_buttonAutoHeight)))
    {
        // 直前に描いたボタンの四角形(左上と右下)を取得し、ボタンの真下にボタンと同じ幅でメニューを開く
        const auto& l_buttonMin = ImGui::GetItemRectMin();
        const auto& l_buttonMax = ImGui::GetItemRectMax();

        const ImVec2 l_popupPos = { l_buttonMin.x, l_buttonMax.y };

        OpenAt(l_popupPos, l_buttonMax.x - l_buttonMin.x);
    }

    return DrawPopup(a_rootLabel, a_categoryNameList);
}
const FWK::TypeINFORegistry::TypeINFO* FWK::Editor::EditorAddMenu::DrawPopup(const std::string_view& a_rootLabel, const std::vector<std::string_view>& a_categoryNameList)
{
    // OpenAtと同じIDでポップアップを探すため、OpenAtと同じくthisをIDスタックへ積む
    // (同じウィンドウにEditorAddMenuが2つあってもIDがぶつからない)
    ImGui::PushID(this);

    // 開いた最初のフレームだけ、OpenAtで決めた位置へメニューの左上を合わせる
    ImGui::SetNextWindowPos(m_popupPos, ImGuiCond_Appearing);

    // 最小と最大の横幅を同じ値にして、OpenAtで決めた幅(ボタンから開いた時はボタンの幅)に固定する
    // (検索で表示する項目が変わってもメニューの幅が揺れないようにするため)
    ImGui::SetNextWindowSizeConstraints(ImVec2(m_popupWidth, k_popupMINHeight), ImVec2(m_popupWidth, k_popupMAXHeight));

    if (!ImGui::BeginPopup(k_popupLabel.data()))
    {
        ImGui::PopID();

        return nullptr;
    }

    DrawSearchInputText();

    DrawCurrentLabel(a_rootLabel);

    ImGui::Separator();

    // 項目が多い時に検索欄までスクロールで隠れないよう、一覧だけを高さ上限つきの子ウィンドウに入れる
    // AutoResizeYを付けると、子ウィンドウの高さは中身に合わせて伸び、上限を超えたらスクロールになる
    ImGui::SetNextWindowSizeConstraints(ImVec2(k_listMINSize, k_listMINSize), ImVec2(k_listMAXWidth, k_listMAXHeight));

    const TypeINFORegistry::TypeINFO* l_selectedTypeINFO = nullptr;

    if (ImGui::BeginChild(k_listChildLabel.data(), Constant::k_imguiRemainingSize, ImGuiChildFlags_AutoResizeY))
    {
        // 検索中はカテゴリーに関係なく全カテゴリーから探し、そうでなければ今いる階層を表示する
        if (IsSearching())
        {
            l_selectedTypeINFO = DrawSearchResultList(a_categoryNameList);
        }
        else if (m_selectedCategoryName.empty())
        {
            DrawCategoryList(a_categoryNameList);
        }
        else
        {
            l_selectedTypeINFO = DrawCategoryTypeList();
        }
    }

    ImGui::EndChild();

    ImGui::EndPopup();

    ImGui::PopID();

    return l_selectedTypeINFO;
}

void FWK::Editor::EditorAddMenu::OpenAt(const ImVec2& a_popupPos, const float a_popupWidth)
{
    // 開くたびにカテゴリー選択の最初の画面から始める
    m_searchInputBuffer.fill(Constant::k_nullCharacter);

    m_selectedCategoryName = {};

    m_popupPos   = a_popupPos;
    m_popupWidth = a_popupWidth;

    // OpenPopupは呼び出した時点のIDスタックでポップアップIDを作るため
    // DrawPopupのBeginPopupと同じくthisを積んだ状態で呼ぶ
    ImGui::PushID   (this);
    ImGui::OpenPopup(k_popupLabel.data());
    ImGui::PopID    ();
}

void FWK::Editor::EditorAddMenu::DrawSearchInputText()
{
    const auto& l_style = ImGui::GetStyle();

    // PushStyleVarでl_styleの中身が書き換わるため、元の余白を先にコピーしておく
    const ImVec2 l_baseFramePadding = l_style.FramePadding;
    const float  l_searchIconWidth  = ImGui::CalcTextSize(k_imguiFontAwesomeSearchIcon.data()).x;

    // InputTextは「入力欄の左端 + FramePadding.x」の位置から文字(カーソル)を書き始める
    // 左の余白を「余白 + アイコンの幅 + 余白」に広げると、空いた所にアイコンを置け、カーソルはアイコンの右から始まる
    const float l_searchFramePaddingX = l_baseFramePadding.x + l_searchIconWidth + l_baseFramePadding.x;

    // メニューを開いた最初のフレームだけ検索欄にフォーカスを当て、開いてすぐ文字を打てるようにする
    if (ImGui::IsWindowAppearing())
    {
        ImGui::SetKeyboardFocusHere(k_keyboardFocusNextItem);
    }

    ImGui::PushStyleVar   (ImGuiStyleVar_FramePadding, ImVec2(l_searchFramePaddingX, l_baseFramePadding.y));
    ImGui::SetNextItemWidth(k_searchInputFullWidth);

    // Hint付きのInputTextは、何も入力されていない時だけ薄い文字(ヒント)を表示する
    ImGui::InputTextWithHint(k_searchInputLabel.data(),
                             k_searchHintLabel.data(),
                             m_searchInputBuffer.data(),
                             m_searchInputBuffer.size());

    ImGui::PopStyleVar();

    // 直前に描いた入力欄の左上を基準にして、広げた左の余白の中へ虫眼鏡アイコンを描く
    // DrawListへ直接描いた物はレイアウト(カーソル位置)に影響しない
    const auto& l_inputTextMin = ImGui::GetItemRectMin();
          auto* l_drawList     = ImGui::GetWindowDrawList();

    const ImVec2 l_searchIconPos = { l_inputTextMin.x + l_baseFramePadding.x, l_inputTextMin.y + l_baseFramePadding.y };

    l_drawList->AddText(l_searchIconPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), k_imguiFontAwesomeSearchIcon.data());
}

void FWK::Editor::EditorAddMenu::DrawCurrentLabel(const std::string_view& a_rootLabel)
{
    if (IsSearching())
    {
        DrawCenteredText(k_searchResultLabel);

        return;
    }

    // カテゴリー選択中は一番上の階層の名前(例 : Component)を表示するだけ
    if (m_selectedCategoryName.empty())
    {
        DrawCenteredText(a_rootLabel);

        return;
    }

    // カテゴリーの中にいる時は「< カテゴリー名」の1行を押せるようにし、押されたらカテゴリー選択へ戻る
    // 文字は描かず、行全体の当たり判定だけをSelectableで作る(「##」以降はIDにだけ使われ表示されない)
    // NoAutoClosePopupsを付けないと、押した瞬間にメニュー全体が閉じてしまう
    if (ImGui::Selectable(k_backSelectableLabel.data(), false, ImGuiSelectableFlags_NoAutoClosePopups))
    {
        m_selectedCategoryName = {};
    }

    // 直前のSelectable(行全体)の四角形を基準にして、<とカテゴリー名をDrawListへ直接描く
    const auto& l_rowMin   = ImGui::GetItemRectMin();
    const auto& l_rowMax   = ImGui::GetItemRectMax();
          auto* l_drawList = ImGui::GetWindowDrawList();

    const auto  l_textColor   = ImGui::GetColorU32(ImGuiCol_Text);
    const float l_rowCenterY  = (l_rowMin.y + l_rowMax.y) * k_centerRatio;
    const float l_arrowHeight = ImGui::GetTextLineHeight() * k_backArrowHeightRatio;
    const float l_arrowLeftX  = l_rowMin.x + ImGui::GetStyle().FramePadding.x;

    // < : 右上の頂点から左の頂点へ、左の頂点から右下の頂点への2本の線(右側は閉じない)
    // フォントの文字ではなく線で描くので、フォントの形に左右されず行の高さに合った大きさで表示される
    const ImVec2 l_arrowLeftPos   = { l_arrowLeftX,                                         l_rowCenterY };
    const ImVec2 l_arrowTopPos    = { l_arrowLeftX + l_arrowHeight * k_backArrowWidthRatio, l_rowCenterY - l_arrowHeight * k_centerRatio };
    const ImVec2 l_arrowBottomPos = { l_arrowLeftX + l_arrowHeight * k_backArrowWidthRatio, l_rowCenterY + l_arrowHeight * k_centerRatio };

    l_drawList->AddLine(l_arrowTopPos,  l_arrowLeftPos,   l_textColor, k_backArrowThickness);
    l_drawList->AddLine(l_arrowLeftPos, l_arrowBottomPos, l_textColor, k_backArrowThickness);

    // カテゴリー名は行の横幅の真ん中に描く
    const float  l_categoryNameWidth = ImGui::CalcTextSize(m_selectedCategoryName.data(), m_selectedCategoryName.data() + m_selectedCategoryName.size()).x;
    const ImVec2 l_categoryNamePos   = { (l_rowMin.x + l_rowMax.x - l_categoryNameWidth) * k_centerRatio, l_rowCenterY - ImGui::GetTextLineHeight() * k_centerRatio };

    l_drawList->AddText(l_categoryNamePos, l_textColor, m_selectedCategoryName.data(), m_selectedCategoryName.data() + m_selectedCategoryName.size());
}
void FWK::Editor::EditorAddMenu::DrawCenteredText(const std::string_view& a_text) const
{
    ShiftCursorToCenter(ImGui::CalcTextSize(a_text.data(), a_text.data() + a_text.size()).x);

    ImGui::TextUnformatted(a_text.data(), a_text.data() + a_text.size());
}

void FWK::Editor::EditorAddMenu::DrawCategoryList(const std::vector<std::string_view>& a_categoryNameList)
{
    for (const auto& l_categoryName : a_categoryNameList)
    {
        // カテゴリーを押しても閉じず、そのカテゴリーの型一覧へ切り替える
        if (ImGui::Selectable(l_categoryName.data(), false, ImGuiSelectableFlags_NoAutoClosePopups))
        {
            m_selectedCategoryName = l_categoryName;
        }
    }
}

const FWK::TypeINFORegistry::TypeINFO* FWK::Editor::EditorAddMenu::DrawSearchResultList(const std::vector<std::string_view>& a_categoryNameList) const
{
    const auto& l_typeINFOTagRegistry = TypeINFOTagRegistry::GetInstance();

    std::vector<const TypeINFORegistry::TypeINFO*> l_hitTypeINFOList = {};

    for (const auto& l_categoryName : a_categoryNameList)
    {
        const auto* l_typeINFOList = l_typeINFOTagRegistry.FindPTRTypeINFOListByTag(l_categoryName);

        if (!l_typeINFOList) { continue; }

        for (const auto* l_typeINFO : *l_typeINFOList)
        {
            if (!IsSearchHit(l_typeINFO->k_name)) { continue; }

            // 1つの型が複数のタグに登録されていることがあるため、追加済みの型は2回入れない
            if (std::ranges::find(l_hitTypeINFOList, l_typeINFO) != l_hitTypeINFOList.end()) { continue; }

            l_hitTypeINFOList.emplace_back(l_typeINFO);
        }
    }

    if (l_hitTypeINFOList.empty())
    {
        ImGui::TextDisabled(k_noSearchHitLabel.data());

        return nullptr;
    }

    return DrawTypeSelectableList(l_hitTypeINFOList);
}
const FWK::TypeINFORegistry::TypeINFO* FWK::Editor::EditorAddMenu::DrawCategoryTypeList() const
{
    const auto& l_typeINFOTagRegistry = TypeINFOTagRegistry::GetInstance              ();
    const auto* l_typeINFOList        = l_typeINFOTagRegistry.FindPTRTypeINFOListByTag(m_selectedCategoryName);

    // タグ名だけ渡されて、まだ1つも型が登録されていないカテゴリー
    if (!l_typeINFOList ||
        l_typeINFOList->empty())
    {
        ImGui::TextDisabled(k_noCategoryTypeLabel.data());

        return nullptr;
    }

    // 並べ替えるためにコピーする(レジストリーの中身は書き換えない)
    auto l_sortTypeINFOList = *l_typeINFOList;

    return DrawTypeSelectableList(l_sortTypeINFOList);
}

const FWK::TypeINFORegistry::TypeINFO* FWK::Editor::EditorAddMenu::DrawTypeSelectableList(std::vector<const TypeINFORegistry::TypeINFO*>& a_typeINFOList) const
{
    // 別々の.cppにある登録の実行順はC++では決まっていないため、名前順に並べて毎回同じ並びで表示する
    std::ranges::sort(a_typeINFOList, {}, &TypeINFORegistry::TypeINFO::k_name);

    const TypeINFORegistry::TypeINFO* l_selectedTypeINFO = nullptr;

    for (const auto* l_typeINFO : a_typeINFOList)
    {
        if (!ImGui::Selectable(l_typeINFO->k_name.data())) { continue; }

        l_selectedTypeINFO = l_typeINFO;

        // 一覧は子ウィンドウの中にあり、Selectableの自動で閉じる処理は子ウィンドウの中では働かないため自分で閉じる
        // CloseCurrentPopupは子ウィンドウの中から呼んでも、外側のポップアップ(このメニュー)を閉じる
        ImGui::CloseCurrentPopup();
    }

    return l_selectedTypeINFO;
}

void FWK::Editor::EditorAddMenu::ShiftCursorToCenter(const float a_itemWidth) const
{
    const float l_availWidth = ImGui::GetContentRegionAvail().x;

    // 残りの横幅の真ん中に幅a_itemWidthの物が来るよう、カーソルを右へずらす
    // 横幅が足りない時は左端から描く(マイナス方向へはずらさない)
    const float l_offsetX = std::max((l_availWidth - a_itemWidth) * k_centerRatio, k_minCursorOffsetX);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + l_offsetX);
}

bool FWK::Editor::EditorAddMenu::IsSearchHit(const std::string_view& a_typeName) const
{
    // バッファは'\0'終端なので、char*からstring_viewを作ると入力された文字の所までになる
    const std::string_view l_searchText = m_searchInputBuffer.data();

    // 型名の中に入力した文字列が含まれていれば一致とする(1文字でも打てば候補が出る)
    // tolowerで大文字小文字を無視して比べるので「camera」でも「Camera」が見つかる
    const auto& l_searchResult = std::ranges::search(a_typeName,
                                                     l_searchText,
                                                     [](const char a_lhsChar, const char a_rhsChar)
                                                     {
                                                         return std::tolower(static_cast<unsigned char>(a_lhsChar)) == std::tolower(static_cast<unsigned char>(a_rhsChar));
                                                     });

    return !l_searchResult.empty();
}

bool FWK::Editor::EditorAddMenu::IsSearching() const
{
    return m_searchInputBuffer.front() != Constant::k_nullCharacter;
}