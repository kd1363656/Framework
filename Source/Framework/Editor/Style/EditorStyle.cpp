#include "EditorStyle.h"

void FWK::Editor::EditorStyle::ApplyUnrealEngineDarkStyle()
{
    // 余白や角丸などの大きさを先に設定し、その後に配色を設定する
    ApplyStyleSize ();
    ApplyStyleColor();
}
void FWK::Editor::EditorStyle::ApplyFont()
{
    auto& l_io = ImGui::GetIO();

    FWK_ASSERT_RETURN_IF(!l_io.Fonts, "フォントが無効です、フォントの適用に失敗しました。");

    // 日本語グリフを含むフォントを読み込む。
    // ImGuiの標準フォントは日本語を持っていないため、
    // UTF-8文字列が正しくても日本語部分が'?'で表示される
    ImFontConfig l_fontConfig = {};

    l_fontConfig.OversampleH = k_fontHorizontalOversample;
    l_fontConfig.OversampleV = k_fontVerticalOversample;
    l_fontConfig.PixelSnapH  = true;

    const auto* const l_japaneseFont = l_io.Fonts->AddFontFromFileTTF(k_fontPath.string().c_str(),
                                                                      k_editorFontSize,
                                                                      &l_fontConfig,
                                                                      l_io.Fonts->GetGlyphRangesJapanese());

    // フォント読み込みに失敗した場合だけ、最低限の標準フォントへ戻す。
    // ただし、この阿合は日本語表示できない
    if (!l_japaneseFont)
    {
        l_io.Fonts->AddFontDefault();
    }

    ImFontConfig l_iconFontConfig = {};

    // 直前に読み込んだCicaへIconGlyphを追加する
    // FontAwesome専用Fontへ切り替える必要がなくなる
    l_iconFontConfig.MergeMode  = true;
    l_iconFontConfig.PixelSnapH = true;

    const auto* const l_iconFong = l_io.Fonts->AddFontFromFileTTF(k_iconFontPath.string().c_str(),
                                                                  k_editorFontSize,
                                                                  &l_iconFontConfig,
                                                                  k_iconGlyphRanges);

    FWK_ASSERT_RETURN_IF(!l_iconFong, "FontAwesomeの読み込みに失敗しました。");
}

void FWK::Editor::EditorStyle::ApplyStyleSize()
{
    auto& l_style = ImGui::GetStyle();

    // ImGui::Begin()で作られる通常ウィンドウ内部の余白。
    // Hierarchy、Inspector、Consoleなどのパネル内の詰まり具合に影響する。
    l_style.WindowPadding = { k_windowPaddingX, k_windowPaddingY };

    // Button、InputText、Combo、Checkboxなどの内側余白。
    // Yを大きくするとボタンや入力欄の高さが増える
    l_style.FramePadding = { k_framePaddingX, k_framePaddingY };

    // UI部品同士の間隔。
    // 小さくすると情報密度が上がるが、見た目が詰まりやすい。
    l_style.ItemSpacing = { k_itemSpacingX, k_itemSpacingY };

    // TreeNode、CollapsingHeaderなどで階層を下げた時の横方向インデント幅。
    // Scene Hierarchyの親子関係の見やすさに影響する。
    l_style.IndentSpacing = k_indentSpacing;

    // スクロールバーの太さ。
    l_style.ScrollbarSize = k_scrollbarSize;

    // 各種Rounding : 角丸の半径
    // UE5はウィンドウ(パネル)は角張っていて、ボタン、入力欄、タブ、ポップアップは少し丸い
    l_style.WindowRounding    = k_windowRounding;
    l_style.ChildRounding     = k_childRounding;
    l_style.FrameRounding     = k_frameRounding;
    l_style.PopupRounding     = k_popupRounding;
    l_style.ScrollbarRounding = k_scrollbarRounding;
    l_style.GrabRounding      = k_grabRounding;
    l_style.TabRounding       = k_tabRounding;

    // 各種BorderSize : 枠線の太さ
    l_style.WindowBorderSize = k_windowBorderSize;
    l_style.ChildBorderSize  = k_childBorderSize;
    l_style.PopupBorderSize  = k_popupBorderSize;
    l_style.FrameBorderSize  = k_frameBorderSize;
    l_style.TabBorderSize    = k_tabBorderSize;

    // 選択中タブの上辺に引く線の太さ
    // UE5ではフォーカス中のパネルのタブの上辺が青く光り、どのパネルを操作中か分かる
    l_style.TabBarOverlineSize = k_tabBarOverlineSize;

    // Dockingされたウィンドウ同士の区切り線の太さ
    l_style.DockingSeparatorSize = k_dockingSeparatorSize;

    // タイトルバー左端の「▼」ボタンを表示しない(UE5のタブには付いていないため)
    l_style.WindowMenuButtonPosition = ImGuiDir_None;

    // ここまで設定したサイズ系パラメータをまとめて倍率変更する。
    // DPI対応をする場合は、将来的にWindow側のDPI値から倍率を決めると良い。
    l_style.ScaleAllSizes(k_editorGlobalScale);
}
void FWK::Editor::EditorStyle::ApplyStyleColor()
{
    auto& l_style = ImGui::GetStyle();

    auto* const l_colorList = l_style.Colors;

    // 文字 ===========================================================================================

    // 通常テキスト色。
    l_colorList[ImGuiCol_Text] = Constant::k_imguiForegroundColor;

    // 無効状態のテキスト色。
    // 使用不可のボタン、説明用の薄い文字などに使われる。
    l_colorList[ImGuiCol_TextDisabled] = Constant::k_imguiForegroundDisabledColor;

    // InputTextなどで文字列を選択した時の背景色。
    l_colorList[ImGuiCol_TextSelectedBg] = Constant::k_imguiAccentTranslucentColor;

    // InputTextの入力位置(カーソル)の色。
    l_colorList[ImGuiCol_InputTextCursor] = Constant::k_imguiForegroundHoverColor;

    // ハイパーリンクの文字色。
    l_colorList[ImGuiCol_TextLink] = Constant::k_imguiAccentHoverColor;

    // 背景 ===========================================================================================

    // 通常ウィンドウの背景色。
    // Docking上の各EditorWindow(Outliner、Details、AssetBrowserなど)の基本背景になる。
    l_colorList[ImGuiCol_WindowBg] = Constant::k_imguiPanelColor;

    // BeginChild()で作る子ウィンドウの背景色。
    // パネルより一段暗くして、へこんだ領域に見せる。
    l_colorList[ImGuiCol_ChildBg] = Constant::k_imguiRecessedColor;

    // Combo、Popup、ContextMenu、Tooltipなどの背景色。
    l_colorList[ImGuiCol_PopupBg] = Constant::k_imguiPanelColor;

    // メニューバー背景色。
    l_colorList[ImGuiCol_MenuBarBg] = Constant::k_imguiBackgroundColor;

    // DockSpace内に何もDockされていない場所の背景色。
    l_colorList[ImGuiCol_DockingEmptyBg] = Constant::k_imguiBackgroundColor;

    // モーダルウィンドウ表示中に後ろを暗くする色。
    l_colorList[ImGuiCol_ModalWindowDimBg] = Constant::k_imguiModalDimColor;

    // 枠線 ===========================================================================================

    // ウィンドウ、Child、Popupなどの枠線色。
    l_colorList[ImGuiCol_Border] = Constant::k_imguiBorderColor;

    // 枠線の影(UE5は影を付けないので透明)。
    l_colorList[ImGuiCol_BorderShadow] = Constant::k_imguiTransparentColor;

    // 区切り線。マウスが乗った時、操作中はアクセントカラーにして操作できることを示す。
    l_colorList[ImGuiCol_Separator]        = Constant::k_imguiBorderColor;
    l_colorList[ImGuiCol_SeparatorHovered] = Constant::k_imguiAccentHoverColor;
    l_colorList[ImGuiCol_SeparatorActive]  = Constant::k_imguiAccentColor;

    // TreeNodeの階層を結ぶ線の色。
    l_colorList[ImGuiCol_TreeLines] = Constant::k_imguiBorderColor;

    // タイトルバー =====================================================================================

    // UE5のタブバーは常に最も暗い色で、フォーカスの有無で色を変えない
    // (どのパネルがフォーカス中かはタブの上辺の青い線で示す)
    l_colorList[ImGuiCol_TitleBg]          = Constant::k_imguiTitleColor;
    l_colorList[ImGuiCol_TitleBgActive]    = Constant::k_imguiTitleColor;
    l_colorList[ImGuiCol_TitleBgCollapsed] = Constant::k_imguiTitleColor;

    // ここから下の「通常、ホバー、押下」の3色は、どの部品でも
    // ホバー = k_imguiHoverColor、押下 = k_imguiPressColor(又はアクセントカラー)にそろえる

    // 入力欄 =========================================================================================

    // InputText、Combo、Checkbox、Slider背景など。
    l_colorList[ImGuiCol_FrameBg]        = Constant::k_imguiInputColor;
    l_colorList[ImGuiCol_FrameBgHovered] = Constant::k_imguiHoverColor;
    l_colorList[ImGuiCol_FrameBgActive]  = Constant::k_imguiPressColor;

    // ボタン =========================================================================================

    l_colorList[ImGuiCol_Button]        = Constant::k_imguiDropdownColor;
    l_colorList[ImGuiCol_ButtonHovered] = Constant::k_imguiHoverColor;
    l_colorList[ImGuiCol_ButtonActive]  = Constant::k_imguiPressColor;

    // TreeNode、Selectable、CollapsingHeader、MenuItem ==================================================

    // 選択色はアイテムごとにIMGUIPushItemHighlightColorsで上書きするため
    // ここでは未選択アイテムの色だけを設定する
    l_colorList[ImGuiCol_Header]        = Constant::k_imguiItemColor;
    l_colorList[ImGuiCol_HeaderHovered] = Constant::k_imguiItemHoveredColor;
    l_colorList[ImGuiCol_HeaderActive]  = Constant::k_imguiItemActiveColor;

    // チェックボックス、スライダー =========================================================================

    // UE5のチェックボックスはチェック時に背景が青くなり、白いチェックマークが付く
    l_colorList[ImGuiCol_CheckMark]          = Constant::k_imguiForegroundHoverColor;
    l_colorList[ImGuiCol_CheckboxSelectedBg] = Constant::k_imguiAccentColor;

    l_colorList[ImGuiCol_SliderGrab]       = Constant::k_imguiAccentColor;
    l_colorList[ImGuiCol_SliderGrabActive] = Constant::k_imguiAccentHoverColor;

    // スクロールバー、リサイズグリップ =======================================================================

    l_colorList[ImGuiCol_ScrollbarBg]          = Constant::k_imguiTransparentColor;
    l_colorList[ImGuiCol_ScrollbarGrab]        = Constant::k_imguiDropdownColor;
    l_colorList[ImGuiCol_ScrollbarGrabHovered] = Constant::k_imguiHoverColor;
    l_colorList[ImGuiCol_ScrollbarGrabActive]  = Constant::k_imguiHover2Color;

    // ウィンドウ右下のリサイズグリップ(UE5には無いので普段は見えないようにする)
    l_colorList[ImGuiCol_ResizeGrip]        = Constant::k_imguiTransparentColor;
    l_colorList[ImGuiCol_ResizeGripHovered] = Constant::k_imguiAccentHoverColor;
    l_colorList[ImGuiCol_ResizeGripActive]  = Constant::k_imguiAccentColor;

    // タブ ===========================================================================================

    // 選択中のタブはパネルと同じ色にして、タブとパネルがつながって見えるようにする
    // 選択されていないタブはタブバーと同じ暗さにする
    l_colorList[ImGuiCol_Tab]                       = Constant::k_imguiTitleColor;
    l_colorList[ImGuiCol_TabHovered]                = Constant::k_imguiHoverColor;
    l_colorList[ImGuiCol_TabSelected]               = Constant::k_imguiPanelColor;
    l_colorList[ImGuiCol_TabSelectedOverline]       = Constant::k_imguiAccentColor;
    l_colorList[ImGuiCol_TabDimmed]                 = Constant::k_imguiTitleColor;
    l_colorList[ImGuiCol_TabDimmedSelected]         = Constant::k_imguiPanelColor;
    l_colorList[ImGuiCol_TabDimmedSelectedOverline] = Constant::k_imguiTransparentColor;

    // Docking、Drag&Drop ==============================================================================

    // Docking中に表示されるプレビュー領域の色。
    l_colorList[ImGuiCol_DockingPreview] = Constant::k_imguiAccentTranslucentColor;

    // Drag&Dropの受け取り先の枠と背景。
    l_colorList[ImGuiCol_DragDropTarget]   = Constant::k_imguiAccentColor;
    l_colorList[ImGuiCol_DragDropTargetBg] = Constant::k_imguiAccentFaintColor;

    // キーボード操作 =====================================================================================

    // キーボード、ゲームパッド操作時のフォーカス枠色。
    l_colorList[ImGuiCol_NavCursor] = Constant::k_imguiAccentColor;

    // Ctrl + Tabでウィンドウを切り替える時の強調色、背景を暗くする色。
    l_colorList[ImGuiCol_NavWindowingHighlight] = Constant::k_imguiAccentColor;
    l_colorList[ImGuiCol_NavWindowingDimBg]     = Constant::k_imguiModalDimColor;

    // 保存していないドキュメントの印(タイトルやタブに付く●)
    l_colorList[ImGuiCol_UnsavedMarker] = Constant::k_imguiForegroundColor;

    // グラフ =========================================================================================

    l_colorList[ImGuiCol_PlotLines]            = Constant::k_imguiAccentColor;
    l_colorList[ImGuiCol_PlotLinesHovered]     = Constant::k_imguiAccentHoverColor;
    l_colorList[ImGuiCol_PlotHistogram]        = Constant::k_imguiAccentColor;
    l_colorList[ImGuiCol_PlotHistogramHovered] = Constant::k_imguiAccentHoverColor;

    // テーブル =======================================================================================

    l_colorList[ImGuiCol_TableHeaderBg]     = Constant::k_imguiHeaderColor;
    l_colorList[ImGuiCol_TableBorderStrong] = Constant::k_imguiBorderColor;
    l_colorList[ImGuiCol_TableBorderLight]  = Constant::k_imguiBorderColor;
    l_colorList[ImGuiCol_TableRowBg]        = Constant::k_imguiTransparentColor;
    l_colorList[ImGuiCol_TableRowBgAlt]     = Constant::k_imguiRecessedColor;
}