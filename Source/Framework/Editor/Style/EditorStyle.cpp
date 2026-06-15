#include "EditorStyle.h"

void FWK::Editor::EditorStyle::ApplySakuraDarkStyle()
{
	ImGuiStyle& l_style = ImGui::GetStyle();

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
	// エディタ用途では細すぎると操作しづらいので、少し太めにする。
	l_style.ScrollbarSize = k_scrollbarSize;

	// 通常ウィンドウの角丸。
	// Docking中心のエディタでは強く効かない場合もある。
	l_style.WindowRounding = k_windowRounding;

	// ImGui::BeginChild()で作る子パネルの角丸。
	// ContentBrowserのサイドバーや、Inspector内の枠付き領域などに効く。
	l_style.ChildRounding = k_childRounding;

	// Button、InputText、Comboなどの角丸。
	// 柔らかいUIにしたい場合は少し大きめにする。
	l_style.FrameRounding = k_frameRounding;

	// ComboのドロップダウンやPopupメニューの角丸。
	l_style.PopupRounding = k_popupRounding;

	// スクロールバーつまみの角丸。
	l_style.ScrollbarRounding = k_scrollbarRounding;

	// SliderやScrollbarのつまみ部分の角丸。
	l_style.GrabRounding = k_grabRounding;

	// Dockingタブ、TabBarの角丸。
	// ViewportタブやContent Browserタブの見た目に影響する。
	l_style.TabRounding = k_tabRounding;

	// 各種BorderSize:
	// 枠線の太さ。
	l_style.WindowBorderSize = k_windowBorderSize;
	l_style.ChildBorderSize  = k_childBorderSize;
	l_style.PopupBorderSize  = k_popupBorderSize;
	l_style.FrameBorderSize  = k_frameBorderSize;
	l_style.TabBorderSize    = k_tabBorderSize;

	// ここまで設定したサイズ系パラメータをまとめて倍率変更する。
	// DPI対応をする場合は、将来的にWindow側のDPI値から倍率を決めると良い。
	l_style.ScaleAllSizes(k_editorGlobalScale);

	auto* const l_colorList = l_style.Colors;

	// 通常テキスト色。
	l_colorList[ImGuiCol_Text] = ConvertEditorColorToIMVEC4(k_textColor);

	// 無効状態のテキスト色。
	// 使用不可のボタン、説明用の薄い文字などに使われる。
	l_colorList[ImGuiCol_TextDisabled] = ConvertEditorColorToIMVEC4(k_textDisabledColor);

	// 通常ウィンドウの背景色。
	// Docking上の各EditorWindowの基本背景になる。
	l_colorList[ImGuiCol_WindowBg] = ConvertEditorColorToIMVEC4(k_backgroundColor);

	// BeginChild()で作る子ウィンドウの背景色。
	// Content Browserの左ツリー、Inspectorの内部枠などに使える。
	l_colorList[ImGuiCol_ChildBg] = ConvertEditorColorToIMVEC4(k_panelColor);

	// Combo、Popup、ContextMenuなどの背景色。
	l_colorList[ImGuiCol_PopupBg] = ConvertEditorColorToIMVEC4(k_panelColor);

	// ウィンドウ、Child、Frame、Tabなどの枠線色。
	l_colorList[ImGuiCol_Border] = ConvertEditorColorToIMVEC4(k_borderColor);

	// 枠線の影。
	l_colorList[ImGuiCol_BorderShadow] = ConvertEditorColorToIMVEC4(k_borderShadowColor);

	// FrameBg:
	// InputText、Combo、Checkbox背景などの通常色。
	l_colorList[ImGuiCol_FrameBg] = ConvertEditorColorToIMVEC4(k_frameColor);

	// InputText、Combo、Checkboxなどにマウスが乗った時の色。
	l_colorList[ImGuiCol_FrameBgHovered] = ConvertEditorColorToIMVEC4(k_frameHoverColor);

	// InputText入力中、Combo選択中などアクティブ状態の色。
	l_colorList[ImGuiCol_FrameBgActive] = ConvertEditorColorToIMVEC4(k_frameActiveColor);

	// 非アクティブなウィンドウタイトルバーの色。
	l_colorList[ImGuiCol_TitleBg] = ConvertEditorColorToIMVEC4(k_panelColor);

	// アクティブなウィンドウタイトルバーの色。
	l_colorList[ImGuiCol_TitleBgActive] = ConvertEditorColorToIMVEC4(k_panelActiveColor);

	// 折り畳まれたウィンドウタイトルバーの色。
	l_colorList[ImGuiCol_TitleBgCollapsed] = ConvertEditorColorToIMVEC4(k_panelColor);

	// メニューバー背景色。
	// File / Edit / View / Tools などを置く場合に効く。
	l_colorList[ImGuiCol_MenuBarBg] = ConvertEditorColorToIMVEC4(k_backgroundColor);

	// スクロールバーの背景。
	l_colorList[ImGuiCol_ScrollbarBg] = ConvertEditorColorToIMVEC4(k_backgroundColor);

	// スクロールバーのつまみ通常色。
	l_colorList[ImGuiCol_ScrollbarGrab] = ConvertEditorColorToIMVEC4(k_panelHoverColor);

	// スクロールバーのつまみにマウスが乗った時の色。
	l_colorList[ImGuiCol_ScrollbarGrabHovered] = ConvertEditorColorToIMVEC4(k_headerHoverColor);

	// スクロールバーをドラッグ中の色。
	l_colorList[ImGuiCol_ScrollbarGrabActive] = ConvertEditorColorToIMVEC4(k_headerActiveColor);

	// Checkboxのチェック、MenuItemのチェックなどの色。
	l_colorList[ImGuiCol_CheckMark] = ConvertEditorColorToIMVEC4(k_accentColor);

	// Sliderのつまみ通常色。
	l_colorList[ImGuiCol_SliderGrab] = ConvertEditorColorToIMVEC4(k_accentColor);

	// Slider操作中のつまみ色。
	l_colorList[ImGuiCol_SliderGrabActive] = ConvertEditorColorToIMVEC4(k_accentActiveColor);

	// 通常ボタン色。
	l_colorList[ImGuiCol_Button] = ConvertEditorColorToIMVEC4(k_buttonColor);

	// ボタンにマウスが乗った時の色。
	l_colorList[ImGuiCol_ButtonHovered] = ConvertEditorColorToIMVEC4(k_buttonHoverColor);

	// ボタン押下中の色。
	l_colorList[ImGuiCol_ButtonActive] = ConvertEditorColorToIMVEC4(k_buttonActiveColor);

	// TreeNode、Selectable、CollapsingHeaderなどの通常背景色。
	// Scene Hierarchyの選択行などにも影響する。
	l_colorList[ImGuiCol_Header] = ConvertEditorColorToIMVEC4(k_headerColor);

	// TreeNode、Selectableなどにマウスが乗った時の色。
	l_colorList[ImGuiCol_HeaderHovered] = ConvertEditorColorToIMVEC4(k_headerHoverColor);

	// TreeNode、Selectableなどをクリック中、または選択中の色。
	l_colorList[ImGuiCol_HeaderActive] = ConvertEditorColorToIMVEC4(k_headerActiveColor);

	// 区切り線の通常色。
	l_colorList[ImGuiCol_Separator] = ConvertEditorColorToIMVEC4(k_borderColor);

	// Splitterなどの区切り線にマウスが乗った時の色。
	l_colorList[ImGuiCol_SeparatorHovered] = ConvertEditorColorToIMVEC4(k_accentHoverColor);

	// Splitterなどの区切り線を操作中の色。
	l_colorList[ImGuiCol_SeparatorActive] = ConvertEditorColorToIMVEC4(k_accentActiveColor);

	// ウィンドウ右下のリサイズグリップ通常色。
	l_colorList[ImGuiCol_ResizeGrip] = ConvertEditorColorToIMVEC4(k_buttonColor);

	// リサイズグリップにマウスが乗った時の色。
	l_colorList[ImGuiCol_ResizeGripHovered] = ConvertEditorColorToIMVEC4(k_accentHoverColor);

	// リサイズ操作中の色。
	l_colorList[ImGuiCol_ResizeGripActive] = ConvertEditorColorToIMVEC4(k_accentActiveColor);

	// 非アクティブなDockingタブの色。
	l_colorList[ImGuiCol_Tab] = ConvertEditorColorToIMVEC4(k_tabColor);

	// Dockingタブにマウスが乗った時の色。
	l_colorList[ImGuiCol_TabHovered] = ConvertEditorColorToIMVEC4(k_tabHoverColor);

	// アクティブなDockingタブの色。
	l_colorList[ImGuiCol_TabActive] = ConvertEditorColorToIMVEC4(k_tabActiveColor);

	// フォーカス外ウィンドウの非アクティブタブ色。
	l_colorList[ImGuiCol_TabUnfocused] = ConvertEditorColorToIMVEC4(k_tabColor);

	// フォーカス外ウィンドウのアクティブタブ色。
	l_colorList[ImGuiCol_TabUnfocusedActive] = ConvertEditorColorToIMVEC4(k_tabActiveColor);

	// Docking中に表示されるプレビュー領域の色。
	l_colorList[ImGuiCol_DockingPreview] = ConvertEditorColorToIMVEC4(k_accentColor);

	// DockSpace内に何もDockされていない場所の背景色。
	l_colorList[ImGuiCol_DockingEmptyBg] = ConvertEditorColorToIMVEC4(k_backgroundColor);

	// ImGui::PlotLines()の線色。
	l_colorList[ImGuiCol_PlotLines] = ConvertEditorColorToIMVEC4(k_accentColor);

	// PlotLinesのホバー色。
	l_colorList[ImGuiCol_PlotLinesHovered] = ConvertEditorColorToIMVEC4(k_accentHoverColor);

	// ImGui::PlotHistogram()の通常色。
	l_colorList[ImGuiCol_PlotHistogram] = ConvertEditorColorToIMVEC4(k_accentColor);

	// PlotHistogramのホバー色。
	l_colorList[ImGuiCol_PlotHistogramHovered] = ConvertEditorColorToIMVEC4(k_accentHoverColor);

	// ImGui::BeginTable()使用時のヘッダー背景色。
	l_colorList[ImGuiCol_TableHeaderBg] = ConvertEditorColorToIMVEC4(k_headerColor);

	// Tableの強い境界線。
	l_colorList[ImGuiCol_TableBorderStrong] = ConvertEditorColorToIMVEC4(k_borderColor);

	// Tableの薄い境界線。
	l_colorList[ImGuiCol_TableBorderLight] = ConvertEditorColorToIMVEC4(k_borderColor);

	// Tableの通常行背景。
	l_colorList[ImGuiCol_TableRowBg] = ConvertEditorColorToIMVEC4(k_backgroundColor);

	// Tableの交互行背景。
	l_colorList[ImGuiCol_TableRowBgAlt] = ConvertEditorColorToIMVEC4(k_panelColor);

	// InputTextなどで文字列を選択した時の背景色。
	l_colorList[ImGuiCol_TextSelectedBg] = ConvertEditorColorToIMVEC4(k_accentActiveColor);

	// キーボード、ゲームパッド操作時のフォーカス枠色。
	l_colorList[ImGuiCol_NavHighlight] = ConvertEditorColorToIMVEC4(k_accentColor);
}
void FWK::Editor::EditorStyle::ApplyDefaultFont()
{
	ImGuiIO& l_io = ImGui::GetIO();

	// ※注意
	// この関数を毎フレーム呼ぶとフォントが重複登録される。
	// 必ずEditor初期化時に1回だけ呼ぶ
	// ImGuiに標準フォントを登録する。
	// 日本語表示やアイコン表示をする場合は、後でここに日本語フォントとFontAwesomeをマージする。
	l_io.Fonts->AddFontDefault();
}

ImVec4 FWK::Editor::EditorStyle::ConvertEditorColorToIMVEC4(const TypeAlias::Math::Color& a_color)
{
	return
	{
		a_color.R(),
		a_color.G(),
		a_color.B(),
		a_color.A()
	};
}