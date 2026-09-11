#include "EditorWindowPaneSplitter.h"


void FWK::Editor::EditorWindowPaneSplitter::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Editor::EditorWindowPaneSplitter::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::EditorWindowPaneSplitter::PreparePaneSize(const ImVec2&                                  a_availableContentRegion, 
	                                                        const Enum::EditorWindowPaneSplitterResizeAxis a_resizeAxis, 
	                                                        const float                                    a_minPrimaryPaneSize, 
	                                                        const float                                    a_minSecondaryPaneSize)
{
	// X軸の場合はWindowの横幅、
	// Y軸の場合はWindowの高さをPane分割に使用する全体サイズとして扱う
	const float l_availableSize = a_resizeAxis == Enum::EditorWindowPaneSplitterResizeAxis::X ? a_availableContentRegion.x : a_availableContentRegion.y;

	// 呼び出し側から0以下の最小サイズが渡された場合でも
	// ImGuiへ不正なPaneサイズを渡さないよう最低描画サイズを保証する
	const float l_minPrimaryPaneSize   = a_minPrimaryPaneSize   < k_minDrawablePaneSize ? k_minDrawablePaneSize : a_minPrimaryPaneSize;
	const float l_minSecondaryPaneSize = a_minSecondaryPaneSize < k_minDrawablePaneSize ? k_minDrawablePaneSize : a_minSecondaryPaneSize;

	// PrimaryPaneを最大まで広げた場合でも
	// PrimaryPane + Splitter + SecondaryPaneのうちのSecondaryPaneの最小サイズを必ず残す
	const float l_maxPrimaryPaneSize = l_availableSize - k_splitterHitThickness - l_minSecondaryPaneSize;

	// Windowそのものが小さすぎて
	// PrimaryPane最小サイズ + Splitter + SecondaryPane最小サイズ
	// のすべてを確保できない場合
	if (l_maxPrimaryPaneSize < l_minPrimaryPaneSize)
	{
		// この場合はPrimaryPaneを可能な限り縮め
		// SecondaryPane側の領域を優先して残す
		m_primaryPaneSize = l_maxPrimaryPaneSize;

		// ImGui::BeginChild()では0.0Fに特別な意味(ウィンドウ霊異記がサイズになる)があるため
		// 完全な0以下にはしない
		if (m_primaryPaneSize < k_minDrawablePaneSize)
		{
			m_primaryPaneSize = k_minDrawablePaneSize;
		}

		return;
	}

	// PrimaryPaneが指定された最小サイズより小さい場合
	if (m_primaryPaneSize < l_minPrimaryPaneSize)
	{
		m_primaryPaneSize = l_minPrimaryPaneSize;

		return;
	}

	// PrimaryPaneが広がりすぎてSecondaryPaneの
	// 最小サイズを浸食している場合
	if (m_primaryPaneSize > l_maxPrimaryPaneSize)
	{
		m_primaryPaneSize = l_maxPrimaryPaneSize;
	}
}

void FWK::Editor::EditorWindowPaneSplitter::Draw(const std::string_view&                        a_label, 
	                                             const ImVec2&                                  a_availableContentRegion, 
	                                             const Enum::EditorWindowPaneSplitterResizeAxis a_resizeAxis, 
	                                             const float                                    a_minPrimaryPaneSize, 
	                                             const float                                    a_minSecondaryPaneSize)
{
	// X軸Resizeかどうかを最初に一度だけ判定する
	const bool l_isXAxis = a_resizeAxis == Enum::EditorWindowPaneSplitterResizeAxis::X;

	if (l_isXAxis)
	{
		// X軸の場合はPrimaryPaneの右隣へSplitterを配置する
		// spacingを0.0Fにすることで
		// PaneとSplitterの間へImGui標準の余白を入れない
		ImGui::SameLine(k_sameLineOffsetFromStartX, k_sameLineSpacing);
	}
	else
	{
		const float l_cursorPOSY = ImGui::GetCursorPosY();
		const float l_spacingY   = ImGui::GetStyle     ().ItemSpacing.y;

		// Y軸の場合、通常のImGuiLayoutでは
		// 直前のItemの下へItemSpacing.yが追加される
		// PaneとSplitterを密着させるため
		// 追加済みのSpacing分だけCursorを戻す
		ImGui::SetCursorPosY(l_cursorPOSY - l_spacingY);
	}

	// Splitterと直交する方向の長さを取得する
	// X軸Resize : Splitterは縦長なのでWindowの高さ
	// Y軸Resize : Splitterは横長なのでWindowの横幅
	const float l_crossAxisSize = l_isXAxis ? a_availableContentRegion.y : a_availableContentRegion.x;

	// Windowが極端に小さい場合でも
	// InvisibleButtonへ0以下のサイズを渡さない
	const float l_drawableCrossAxisSize = l_crossAxisSize < k_minDrawableSplitterSize ? k_minDrawableSplitterSize                                 : l_crossAxisSize;
	const auto& l_splitterSize          = l_isXAxis                                   ? ImVec2{ k_splitterHitThickness, l_drawableCrossAxisSize } : ImVec2{ l_drawableCrossAxisSize, k_splitterHitThickness };

	// InvisibleButton()は画面上にButtonを表示せず
	// Mouse入力だけを受け取るImGuiItemを作成する
	// SplitterのDrag判定領域として使用する
	ImGui::InvisibleButton(a_label.data(), l_splitterSize);

	const bool l_isSplitterHovered = ImGui::IsItemHovered();
	const bool l_isSplitterActive  = ImGui::IsItemActive ();

	// InvisibleButtonの左上・左下座標
	// ImGui側が値で返すため、
	// 8ByteのImVec2はconst参照で受けてTemporaryのLifetimeを
	// このScopeまで延期している
	const auto& l_splitterMIN = ImGui::GetItemRectMin();
	const auto& l_splitterMAX = ImGui::GetItemRectMax();

	// GetWindowDrawList()が返すImDrawList*はImGui所有
	// この関数内だけで一時的に参照し、保持はしない
	auto* l_drawList = ImGui::GetWindowDrawList();

	if (!l_drawList) 
	{
		// 通常はImGui標準Separator色
		ImGuiCol l_splitterColor = ImGuiCol_Separator;

		// Drag中はActive色を使用する
		if (l_isSplitterActive)
		{
			l_splitterColor = ImGuiCol_SeparatorActive;
		}
		// MouseがSplitter上にある場合はHovered色を使用する
		else if (l_isSplitterHovered)
		{
			l_splitterColor = ImGuiCol_SeparatorHovered;
		}

		// InvisibleButton()はMouse判定用として6Pixel確保しているが
		// 見た目まで6PixelにするとSplitterが太く見える
		// そのためHit領域の中央へ1Pixelの境界線だけ描画する
		if (l_isXAxis)
		{
			// X軸Resizeでは縦Splitterになるため、
			// Hit領域中央のX座標を求める
			const float l_splitterCenterX = (l_splitterMIN.x + l_splitterMAX.x) / k_centerDivisor;

			l_drawList->AddLine({ l_splitterCenterX, l_splitterMIN.y }, 
				                { l_splitterCenterX, l_splitterMAX.y }, 
				                ImGui::GetColorU32(l_splitterColor), 
				                k_splitterLineThickness);
		}
		else
		{
			// Y軸Resizeでは横Splitterになるため、
			// Hit領域中央のY座標を求める
			const float l_splitterCenterY = (l_splitterMIN.y + l_splitterMAX.y) / k_centerDivisor;

			l_drawList->AddLine({ l_splitterMIN.x, l_splitterCenterY },
				                { l_splitterMAX.x, l_splitterCenterY },
				                 ImGui::GetColorU32(l_splitterColor), 
				                 k_splitterLineThickness);
		}
	}
	
	// Hover中またはDrag中は、
	// Resize方向に対応するMouseCursorへ変更する
	if (l_isSplitterHovered || 
		l_isSplitterActive)
	{
		ImGui::SetMouseCursor(l_isXAxis ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
	}

	// InvisibleButtonを左MouseButtonで掴んでいる間
	if (l_isSplitterActive)
	{
		// ImGui::GetIO()はImGuiが所有するIO情報への参照
		const auto& l_io = ImGui::GetIO();

		// X軸ならMouseの横移動量
		// Y軸ならMouseの縦移動量
		const float l_mouseDelta = l_isXAxis ? l_io.MouseDelta.x : l_io.MouseDelta.y;

		m_primaryPaneSize += l_mouseDelta;

		// Dragによって最小 / 最大サイズを超えないよう
		// 変更直後に補正する
		PreparePaneSize(a_availableContentRegion,
		                a_resizeAxis,
		                a_minPrimaryPaneSize,
			            a_minSecondaryPaneSize);
	}

	if (l_isXAxis)
	{
		// SecondaryPaneをSplitterの右隣へ配置する
		ImGui::SameLine(k_sameLineOffsetFromStartX, k_sameLineSpacing);
	}
	else
	{
		// InvisibleButton描画後にもItemSpacing.yが追加されるため
		// SecondaryPaneをSplitter直下へ密着させるようCursorを戻す
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);
	}
}