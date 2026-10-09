#include "DetailsEditorGameObjectAddComponentPopupDrawer.h"

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::Draw(const std::weak_ptr<GameObject>& a_gameObject,
                                                                       const std::string_view&          a_popupLabel,
                                                                       const ImVec2&                    a_popupPosition,
                                                                       const float                      a_popupWidth)
{
    // ポップアップを「コンポーネント追加」ボタンの真下に、ボタンと同じ幅で開く
    // 高さは0.0Fを渡すと中身に合わせて自動で決まる
    ImGui::SetNextWindowPos (a_popupPosition);
    ImGui::SetNextWindowSize(ImVec2{ a_popupWidth, k_autoFitPopupHeight });

    if (!ImGui::BeginPopup(a_popupLabel.data())) { return; }

    // ImGui::IsWindowAppearing : ポップアップが開いた最初のフレームだけtrueを返す
    // 前回開いたときの検索文字列や選択中の種類が残らないよう、開くたびに初期状態へ戻す
    if (ImGui::IsWindowAppearing())
    {
        Reset();
    }

    // 検索欄の入力で検索中かどうかが変わる前の状態を覚えておく
    // 例 : Backspaceで最後の1文字を消したフレームに、種類の一覧へ戻る処理まで動いてしまわないようにするため
    const bool l_wasSearching = IsSearching();

    DrawSearchBar();
    DrawHeader   ();

    ImGui::Separator();

    // 今の状態(検索中 / 種類を選ぶ前 / 種類を選んだ後)に合わせて、一覧に並べる文字列を集める
    const auto& l_itemList = FetchVALItemList(a_gameObject);

    // 検索文字列が変わって一覧が短くなった場合に、ハイライトの位置が一覧の外を指さないようにする
    if (!l_itemList.empty() &&
        m_highlightIndex >= l_itemList.size())
    {
        m_highlightIndex = l_itemList.size() - k_highlightStep;
    }

    HandleShortcut(l_itemList, a_gameObject, l_wasSearching);

    DrawItemList(l_itemList, a_gameObject);

    ImGui::EndPopup();
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::Reset()
{
    // 検索文字列・選択中の種類・ハイライトの位置を初期状態に戻す
    m_searchBuffer.fill(Constant::k_nullCharacter);
    m_selectedTag.clear();

    m_highlightIndex = k_initialHighlightIndex;

    // 開いてすぐに文字を入力できるよう、次の描画で検索欄へフォーカスする
    m_shouldFocusSearchBar    = true;
    m_shouldScrollToHighlight = false;
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawSearchBar()
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
        const float l_clearButtonWidth = ImGui::CalcTextSize(k_clearSearchButtonLabel.data(), nullptr, true).x + l_style.FramePadding.x * k_framePaddingBothSidesNUM + l_style.ItemSpacing.x;

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
                                 k_searchHintLabel.data(),
                                 m_searchBuffer.data(),
                                 m_searchBuffer.size()))
    {
        // 一覧の中身が変わるため、ハイライトを先頭へ戻す
        m_highlightIndex = k_initialHighlightIndex;
    }

    if (!l_isSearching) { return; }

    ImGui::SameLine();

    // 文字列とは独立した×ボタン
    // クリックしたら入力内容をすべて消し、続けて入力できるよう検索欄へフォーカスを戻す
    if (ImGui::Button(k_clearSearchButtonLabel.data()))
    {
        m_searchBuffer.fill(Constant::k_nullCharacter);

        m_highlightIndex       = k_initialHighlightIndex;
        m_shouldFocusSearchBar = true;
    }
}
void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawHeader()
{
    // 検索中は種類に関係なく、すべての種類から一致したコンポーネントを並べる
    // そのため戻る矢印は出さず、「検索結果」だけを表示する
    if (IsSearching())
    {
        DrawCenteredText(k_searchResultLabel);

        return;
    }

    // まだ種類を選んでいなければ「コンポーネントの種類」を表示する
    if (m_selectedTag.empty())
    {
        DrawCenteredText(k_componentTagLabel);

        return;
    }

    // 種類を選んだ後は、左端の三角形(<)と選んだ種類の名前を含めた、行全体を1つの当たり判定にする
    // 三角形だけを押せるボタンにすると、名前を押しても反応しないため、
    // 左端から右端までの1つのSelectableで当たり判定を作り、クリックで種類の一覧へ戻る
    // 幅を指定しない(0.0F)と、行の右端まで広がるため、名前の右側を押しても反応する
    // 三角形と名前は、このSelectableの上に描く
    // ImGuiSelectableFlags_NoAutoClosePopups : ポップアップ直下のSelectableはクリックでポップアップを閉じるため、閉じないようにする
    // 高さ(0.0F)は、通常の文字と同じ高さになる。種類を選ぶ前の見出し(文字だけ)と高さが変わらないため、一覧の位置がずれない
    if (ImGui::Selectable(k_backHeaderLabel.data(), false, ImGuiSelectableFlags_NoAutoClosePopups))
    {
        ClearSelectedTag();
    }

    // 直前の項目(Selectable)の左端に三角形を描く
    DrawBackArrow();

    // 同じ行に、選んだ種類の名前を三角形とは独立した位置(ポップアップの真ん中)へ表示する
    // 名前は項目ではなく文字なので、下のSelectableのクリックを邪魔しない
    ImGui::SameLine();

    DrawCenteredText(m_selectedTag);
}
void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawItemList(const std::vector<std::string>& a_itemList, const std::weak_ptr<GameObject>& a_gameObject)
{
    // 一覧は高さを固定した子ウィンドウに並べ、項目が多いときはスクロールできるようにする
    // ImGui::BeginChildはfalseを返した場合もEndChildを呼ぶ必要がある
    if (!ImGui::BeginChild(k_itemListChildLabel.data(), k_itemListChildSize))
    {
        ImGui::EndChild();

        return;
    }

    if (a_itemList.empty())
    {
        ImGui::TextDisabled(k_noAddableComponentLabel.data());

        ImGui::EndChild();

        return;
    }

    // 種類の一覧(検索中でなく、まだ種類を選んでいない状態)かどうか
    // 種類の行には、中のコンポーネントの一覧へ進めることを表す">"を右端に描く
    const bool l_isTagList = !IsSearching() &&
                             m_selectedTag.empty();

    for (std::size_t l_itemIndex = 0ULL; l_itemIndex < a_itemList.size(); ++l_itemIndex)
    {
        const auto& l_item          = a_itemList[l_itemIndex];
        const bool  l_isHighlighted = l_itemIndex == m_highlightIndex;

        // 検索結果では、同じコンポーネントが複数の種類に登録されている場合に同じ文字列が並ぶことがある
        // ImGuiのIDは文字列から作られるため、添字をIDに混ぜて重ならないようにする
        // ※注意 : 項目のアドレスをIDにしてはいけない
        //          一覧はフレームごとに作り直すため、アドレスが変わるとIDも変わる
        //          クリックはマウスを押したフレームと離したフレームで同じIDである必要があり、IDが変わると反応しなくなる
        ImGui::PushID(static_cast<int>(l_itemIndex));

        // ImGuiSelectableFlags_NoAutoClosePopups : クリックしてもポップアップを閉じない
        // 種類を選んだときは、ポップアップを開いたままコンポーネントの一覧へ進むため
        const bool l_isClicked = ImGui::Selectable(l_item.c_str(), l_isHighlighted, ImGuiSelectableFlags_NoAutoClosePopups);

        ImGui::PopID();

        // 直前に描画した項目(Selectable)の右端に">"を描く
        // GetItemRectMin / GetItemRectMaxが直前の項目を指すため、Selectableの直後に呼ぶ
        if (l_isTagList)
        {
            DrawTagArrow();
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

        DecideItem(a_gameObject, l_item);

        // 決定した時点で一覧の中身が変わるため、このフレームの描画はここで終える
        break;
    }

    ImGui::EndChild();
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawSearchIcon() const
{
    // 虫眼鏡のアイコンは、通常の文字の大きさだと検索欄(InputText)に対して小さく見えるため、少し大きく描く
    // 文字として並べると、大きくした分だけ行の高さと位置がずれるため、
    // 描画リストで検索欄(フレーム)の高さの真ん中に描き、アイコンの幅と検索欄の高さの空きだけをレイアウトに確保する
          auto* l_font         = ImGui::GetFont();
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
          auto* l_drawList       = ImGui::GetWindowDrawList();

    if (!l_drawList) { return; }

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
void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawTagArrow() const
{
    // 直前に描画した項目(Selectable)の右端に、中へ進めることを表すFontAwesomeの右向きの三角形(>)を描く
    // 種類の名前とは独立した位置に描くため、描画リストへ直接描く
    const auto& l_itemMIN   = ImGui::GetItemRectMin();
    const auto& l_itemMAX   = ImGui::GetItemRectMax();
    const auto& l_arrowSize = ImGui::CalcTextSize  (k_imguiFontAwesomeCaretRightIcon.data());

    // 横 : 項目の右端から、矢印の幅と左右の余白(FramePadding)だけ内側
    // 縦 : 項目の高さの真ん中
    const auto& l_style         = ImGui::GetStyle();
    const auto& l_arrowPosition = ImVec2{ l_itemMAX.x - l_arrowSize.x - l_style.FramePadding.x, l_itemMIN.y + (l_itemMAX.y - l_itemMIN.y - l_arrowSize.y) * Constant::k_halfMagnification };
          auto* l_drawList      = ImGui::GetWindowDrawList();

    if (!l_drawList) { return; }

    l_drawList->AddText(l_arrowPosition, ImGui::GetColorU32(ImGuiCol_TextDisabled), k_imguiFontAwesomeCaretRightIcon.data());
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawBackArrow() const
{
    // 直前に描画した項目(Selectable)の左端に、一覧へ戻ることを表すFontAwesomeの左向きの三角形(<)を描く
    // DrawTagArrowとは左右が逆。項目の左端から余白(FramePadding)だけ内側に、高さの真ん中で描く
    const auto& l_itemMIN   = ImGui::GetItemRectMin();
    const auto& l_itemMAX   = ImGui::GetItemRectMax();
    const auto& l_arrowSize = ImGui::CalcTextSize  (k_imguiFontAwesomeCaretLeftIcon.data());

    const auto& l_style         = ImGui::GetStyle();
    const auto& l_arrowPosition = ImVec2{ l_itemMIN.x + l_style.FramePadding.x, l_itemMIN.y + (l_itemMAX.y - l_itemMIN.y - l_arrowSize.y) * Constant::k_halfMagnification };

    ImGui::GetWindowDrawList()->AddText(l_arrowPosition, ImGui::GetColorU32(ImGuiCol_TextDisabled), k_imguiFontAwesomeCaretLeftIcon.data());
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DrawCenteredText(const std::string_view& a_text) const
{
    // ポップアップの幅の真ん中に文字列を置く
    // SameLineで矢印の後ろに描画した場合も、矢印の位置とは関係なく真ん中に置くため、矢印と文字列は続けて並ばない
    const float l_textWidth   = ImGui::CalcTextSize  (a_text.data(), a_text.data() + a_text.size()).x;
    const float l_windowWidth = ImGui::GetWindowWidth();

    ImGui::SetCursorPosX  ((l_windowWidth - l_textWidth) * Constant::k_halfMagnification);
    ImGui::TextUnformatted(a_text.data(), a_text.data() + a_text.size());
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::HandleShortcut(const std::vector<std::string>& a_itemList, const std::weak_ptr<GameObject>& a_gameObject, const bool a_wasSearching)
{
    // ←とBackspaceで種類の一覧へ戻る
    // 検索欄に文字が入っているときは、カーソルの移動や文字の削除に使うため戻らない
    if ((ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ||
        ImGui::IsKeyPressed(ImGuiKey_Backspace)) &&
        !a_wasSearching &&
        !m_selectedTag.empty())
    {
        ClearSelectedTag();

        return;
    }

    if (a_itemList.empty()) { return; }

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
    const bool l_isEnterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter, false) ||
                                  ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, false);

    // →は種類の一覧で、ハイライト中の種類へ進むときだけ使う
    const bool l_isEnterTagPressed = !a_wasSearching &&
                                     m_selectedTag.empty() &&
                                     ImGui::IsKeyPressed(ImGuiKey_RightArrow, false);

    if (!l_isEnterPressed &&
        !l_isEnterTagPressed)
    {
        return;
    }

    DecideItem(a_gameObject, a_itemList[m_highlightIndex]);
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::DecideItem(const std::weak_ptr<GameObject>& a_gameObject, const std::string& a_item)
{
    // 種類の一覧で決定したときは、その種類のコンポーネントの一覧へ進む
    if (!IsSearching() &&
        m_selectedTag.empty())
    {
        m_selectedTag = a_item;

        m_highlightIndex = k_initialHighlightIndex;

        // 検索欄でEnterを押すと検索欄のフォーカスが外れるため、続けて入力できるよう戻す
        m_shouldFocusSearchBar = true;

        return;
    }

    // コンポーネントの一覧(検索結果を含む)で決定したときは、コンポーネントを追加してポップアップを閉じる
    AddComponent(a_gameObject, a_item);

    ImGui::CloseCurrentPopup();
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::ClearSelectedTag()
{
    // 選んでいた種類を消して、種類の一覧の状態へ戻す
    m_selectedTag.clear();

    m_highlightIndex       = k_initialHighlightIndex;
    m_shouldFocusSearchBar = true;
}

void FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::AddComponent(const std::weak_ptr<GameObject>& a_gameObject, const std::string& a_componentTypeName) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return; }

    // 型名からコンポーネントを生成する
    // FWK_REGISTER_FACTORY_METHODでGameObjectComponentSharedFactoryへ登録した型だけが生成できる
    const auto& l_componentFactory = TypeAlias::GameObjectComponentSharedFactory::GetInstance();
    const auto& l_component        = l_componentFactory.Create                               (a_componentTypeName);

    if (!l_component)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "コンポーネントの生成に失敗したため、コンポーネントの追加に失敗しました。\nComponentType : {}", a_componentTypeName);

        return;
    }

    // 生成したコンポーネントを初期化する
    // ※注意 : INITはOwnerとUUIDを空に戻すため、必ずSetOwnerとAddComponentより前に呼ぶ
    l_component->INIT();

    // 追加先のGameObjectをOwnerとして設定する
    // 引数のweak_ptrをそのまま代入するため、追加先のGameObjectを共有(所有)しない
    l_component->SetOwner(a_gameObject);

    // UUIDの発行・Unique/Multiのマップへの振り分けはAddComponentが行う
    if (auto& l_componentContainer = l_gameObject->GetMutableREFComponentContainer();
        !l_componentContainer.AddComponent(l_component))
    {
        return;
    }

    // シーンを読み込んだときと同じく、追加した後にPostDeserializeを呼んで初期設定を終える
    // 例 : カメラコンポーネントはここでカメラの行列を準備する
    l_component->PostDeserialize();

    // 追加を終えてから、履歴へ積む
    // コマンドがshared_ptrを持つため、Undoでコンテナから外れたコンポーネントもRedoまで消えない
    auto& l_editorManager  = EditorManager::GetInstance                 ();
    auto& l_undoRedoSystem = l_editorManager.GetMutableREFUndoRedoSystem();

    l_undoRedoSystem.PushUndoCommand<AddGameObjectComponentCommand>(l_component, l_gameObject->GetREFSceneInstanceUUID());
}

bool FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::IsSearching() const
{
    // 配列の先頭が終端文字でなければ、1文字以上入力されている
    return m_searchBuffer.front() != Constant::k_nullCharacter;
}
bool FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::IsMatchedSearchText(const std::string& a_componentTypeName) const
{
    const auto& l_searchText = std::string_view{ m_searchBuffer.data() };

    // 型名の中に検索文字列が含まれているかを、大文字と小文字を区別せずに調べる
    // std::ranges::search : 見つかった範囲を返し、見つからなければ空の範囲を返す
    // 例 : "camera"で検索すると"GameObjectCameraComponent"が一致する
    const auto& l_matchedRange = std::ranges::search(a_componentTypeName,
                                                     l_searchText,
                                                     [](const char a_left, const char a_right)
                                                     {
                                                         return std::tolower(static_cast<unsigned char>(a_left)) == std::tolower(static_cast<unsigned char>(a_right));
                                                     });

    return !l_matchedRange.empty();
}

bool FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::CanAddComponent(const std::string& a_componentTypeName, const GameObjectComponentContainer& a_componentContainer) const
{
    const auto& l_componentSmartPointerVectorList = a_componentContainer.GetREFComponentSmartPointerVectorList();
    const auto& l_componentDataList               = l_componentSmartPointerVectorList.GetREFElementDataList   ();

    // 複数持てないコンポーネントを、すでに追加している場合は追加できない
    // 複数持てるコンポーネント(IsAllowMultipleがtrue)は、何個あっても追加できる
    return std::ranges::none_of(l_componentDataList,
                                [&a_componentTypeName](const auto& a_componentData)
                                {
                                    const auto& l_component = a_componentData.m_type;

                                    return l_component &&
                                           !l_component->IsAllowMultiple() &&
                                           l_component->GetREFRuntimeTypeINFO().k_name == a_componentTypeName;
                                });
}

std::vector<std::string> FWK::Editor::DetailsEditorGameObjectAddComponentPopupDrawer::FetchVALItemList(const std::weak_ptr<GameObject>& a_gameObject) const
{
    const auto& l_gameObject = a_gameObject.lock();

    if (!l_gameObject) { return {}; }

          std::vector<std::string> l_itemList           = {};
    const auto&                    l_taggedFactory      = GameObjectComponentTaggedFactory::GetInstance     ();
    const auto&                    l_taggedComponentMap = l_taggedFactory.GetREFTaggedGameObjectComponentMap();
    const auto&                    l_componentContainer = l_gameObject->GetREFComponentContainer            ();

    // 検索中は、すべての種類のコンポーネントから、型名が一致して追加できるものを集める
    if (IsSearching())
    {
        for (const auto& [l_tag, l_componentTypeNameList] : l_taggedComponentMap)
        {
            for (const auto& l_componentTypeName : l_componentTypeNameList)
            {
                if (!IsMatchedSearchText(l_componentTypeName) ||
                    !CanAddComponent(l_componentTypeName, l_componentContainer))
                {
                    continue;
                }

                l_itemList.emplace_back(l_componentTypeName);
            }
        }

        return l_itemList;
    }

    // 種類を選ぶ前は、種類(マップのキー)の一覧を集める
    if (m_selectedTag.empty())
    {
        l_itemList.reserve(l_taggedComponentMap.size());

        for (const auto& [l_tag, l_componentTypeNameList] : l_taggedComponentMap)
        {
            l_itemList.emplace_back(l_tag);
        }

        return l_itemList;
    }

    // 種類を選んだ後は、その種類に登録されているコンポーネントのうち、追加できるものを集める
    const auto& l_taggedComponentITR = l_taggedComponentMap.find(m_selectedTag);

    if (l_taggedComponentITR == l_taggedComponentMap.end()) { return l_itemList; }

    for (const auto& l_componentTypeName : l_taggedComponentITR->second)
    {
        if (!CanAddComponent(l_componentTypeName, l_componentContainer)) { continue; }

        l_itemList.emplace_back(l_componentTypeName);
    }

    return l_itemList;
}