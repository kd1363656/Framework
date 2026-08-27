#include "InputComponentInspector.h"

void FWK::InputComponentInspector::INIT()
{
	m_nodeEditor.INIT();
}

void FWK::InputComponentInspector::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::InputComponentInspector::EditInspector(InputComponent& a_inputComponent)
{
	// DetailsEditorWindow内部でNodeEditorが
	// 必要以上に縦へ広がらないよう固定高さのChildを作る
	ImGui::BeginChild(k_nodeEditorLabel.data(), Constant::k_defaultNodeEditorCanvasSize);

	if (!m_nodeEditor.BeginDraw())
	{
		ImGui::EndChild();

		return;
	}

	// 前フレームに右クリックによってOpenPopupされたPopupをNodeEditorが有効な状態で描画する
	DrawNodeEditorContextMenu(a_inputComponent);

	// InputComponentの処理開始位置
	DrawStartNode();

	// 右クリックで追加されたComponentEvent条件を描画する
	DrawExecutionConditionNodeList(a_inputComponent);

	// MiniMapは全Node/Linkを指定した後
	// EndNodeEditorより前に描画する必要がある
	ImNodes::MiniMap(k_minMapSizeFraction, ImNodesMiniMapLocation_BottomRight);

	const bool l_isEditorHovered = ImNodes::IsEditorHovered();

	m_nodeEditor.EndDraw();

	// IsNodesHovered(9はEndNodeEditor後に問い合わせるため
	// ContextMenu処理はNodeEditor描画終了後に行う
	PrepareNodeEditorContextMenu(l_isEditorHovered, a_inputComponent);

	ImGui::EndChild();
}

nlohmann::json FWK::InputComponentInspector::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::InputComponentInspector::DrawStartNode()
{
	      auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();
	const bool  l_isCreatedThisFrame  = !m_startNode.FetchVALIsCreated               ();

	// StartはInputComponentに必ず存在する固定Nodeなので、
	// まだNodeIDがなければ自動作成する
	if (!m_startNode.FetchVALIsCreated() &&
		!m_startNode.ApplyNodeID(l_nodeEditorAllocator))
	{
		return;
	}

	// Startは前段Nodeを持たないためInputPinは不要
	// 次のNodeへ実行を渡すためのOutputPinを1個だけ持つ
	if (m_startNode.GetREFOutputPinIDList().empty())
	{
		if (!m_startNode.AddOutputPinID(l_nodeEditorAllocator))
		{
			m_startNode.Release(l_nodeEditorAllocator);

			return;
		}
	}

	const auto l_nodeID = m_startNode.GetVALNodeID();

	// 新しく生成したStartだけ初期位置を設定する
	// 毎Frame位置を設定する
	// ユーザーがドラッグしても元の位置へも取って暇う
	if (l_isCreatedThisFrame) 
	{
		ImNodes::SetNodeGridSpacePos(l_nodeID, k_startNodeInitialPosition);
	}

	ImNodes::BeginNode        (l_nodeID);
	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted    (k_startNodeLabel.data());
	ImNodes::EndNodeTitleBar  ();

	const auto& l_outputPinIDList = m_startNode.GetREFOutputPinIDList();

	ImNodes::BeginOutputAttribute(l_outputPinIDList.front());
	ImGui::TextUnformatted       (k_outputPinLabel.data());

	ImNodes::EndOutputAttribute();
	ImNodes::EndNode           ();
}
void FWK::InputComponentInspector::DrawExecutionConditionNodeList(InputComponent& a_inputComponent)
{
	      auto& l_executionConditionList = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList    ();
	const auto& l_componentEventRegistry = Utility::StringValueBidirectionalRegistry<Enum::ComponentEvent>::GetInstance();
	const auto& l_eventLabeRegistry      = Utility::StringValueBidirectionalRegistry<Enum::EventLane>::GetInstance     ();

	for (auto& l_executionCondition : l_executionConditionList)
	{
		if (!PrepareExecutionConditionNode(l_executionCondition)) { continue; }

		const auto& l_node   = l_executionCondition.m_editorNodeEditor;
		const auto  l_nodeID = l_node.GetVALNodeID();

		const auto& l_componentEventName = l_componentEventRegistry.FindVALKeyByValue(l_executionCondition.m_receiveComponentEvent);
		const auto& l_eventLaneName      = l_eventLabeRegistry.FindVALKeyByValue     (l_executionCondition.m_checkEventLane);

		// 各ConditionNode内部には同名Widgetが存在するため
		// NodeIDをImGui側のIDScopeとして使用する
		ImGui::PushID     (l_nodeID);
		ImNodes::BeginNode(l_nodeID);

		ImNodes::BeginNodeTitleBar();

		ImGui::TextUnformatted(l_componentEventName.data(), l_componentEventName.data() + l_componentEventName.size());

		ImNodes::EndNodeTitleBar();

		const auto& l_inputPinIDList = l_node.GetREFInputPinIDList();

		ImNodes::BeginInputAttribute(l_inputPinIDList.front());
		ImGui::TextUnformatted      (k_inputLabel.data());
		ImNodes::EndInputAttribute  ();
		ImGui::TextUnformatted      (k_eventLaneLabel.data());

		if (ImGui::BeginCombo(k_eventLaneComboBoxLabel.data(), l_eventLaneName.data()))
		{
			for (const auto& [l_key, l_value] : l_eventLabeRegistry.GetREFStringToValueMap())
			{
				const bool l_isSelected = l_executionCondition.m_checkEventLane == l_value;

				if (ImGui::Selectable(l_key.c_str(), l_isSelected)) 
				{
					l_executionCondition.m_checkEventLane = l_value;
				}

				if (l_isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Checkbox(k_expectedResultLabel.data(), &l_executionCondition.m_expectedObserverResult);

		const auto& l_outputPinIDList = l_node.GetREFOutputPinIDList();

		ImNodes::BeginOutputAttribute(l_outputPinIDList.front(), ImNodesPinShape_TriangleFilled);
		ImGui::TextUnformatted       (k_outputPinLabel.data());
		ImNodes::EndOutputAttribute  ();
		ImNodes::EndNode             ();
		ImGui::PopID                 ();
	}
}

void FWK::InputComponentInspector::DrawNodeEditorContextMenu(InputComponent& a_inputComponent)
{
	// PrepareNodeEditorContextMenu(9でOpenPopupしたときと
	// 完全に同じIDScopeを使用する必要がある
	ImGui::PushID(std::addressof(a_inputComponent));

	switch (m_contextMenuRequest)
	{
		case ContextMenuRequest::CreateNode: 
		{

			ImGui::OpenPopup(k_addNodeContextMenuLabel.data());
		}
		break;

		case ContextMenuRequest::Node:
		{
			ImGui::OpenPopup(k_nodeContextMenuLabel.data());
		}
		break;

		default:
		break;
	}

	// OpenPopupは一回だけ要求すればいいので
	// このFrameで要求状態を解除
	m_contextMenuRequest = ContextMenuRequest::None;

	DrawCreateNodeContextMenu   (a_inputComponent);
	DrawNodeContextMenu         (a_inputComponent);
	ImGui::PopID                ();
}
void FWK::InputComponentInspector::PrepareNodeEditorContextMenu(const bool a_isEditorHovered, InputComponent & a_inputComponent)
{
	TypeAlias::NodeEditorID l_hoveredNodeID = Constant::k_invalidNodeEditorID;

	// ImNodes仕様上、子の判定はEndNodeEditor後に行う
	const bool l_isNodeHovered = ImNodes::IsNodeHovered(&l_hoveredNodeID);

	// NodeEditor上で右クリックされていないなら
	// ContextMenuは開く必要はない
	if (!ImGui::IsMouseClicked(ImGuiMouseButton_Right)) { return; }

	// NodeにもeditorCanvasにもマウスが乗っていないなら
	// このInputComponent用のContextMenuではない
	if (!l_isNodeHovered &&
		!a_isEditorHovered)
	{
		return;
	}

	if (l_isNodeHovered)
	{
		// Node上で右クリックされた場合
		// Popupが実際に描画されるのは次Frameなので、
		// どのNodeだったかだけ保持しておく
		m_contextMenuTargetNodeID = l_hoveredNodeID;

		m_contextMenuRequest = ContextMenuRequest::Node;

		return;
	}

	// 空白部分を右クリックした瞬間の位置を保存する
	// Popup自体はは次Frameなので
	// GetMousePoseOpeningCurrentPopup()ではなく
	// この時点のMousePosを使う
	m_createNodeScreenSpacePosition = ImGui::GetMousePos();
	m_contextMenuRequest            = ContextMenuRequest::CreateNode;
}

void FWK::InputComponentInspector::DrawCreateNodeContextMenu(InputComponent& a_inputComponent)
{
	// ポップアップのサイズ指定
	ImGui::SetNextWindowSize(k_createNodeContextMenuSize, ImGuiCond_Always);

	if (!ImGui::BeginPopup(k_addNodeContextMenuLabel.data())) { return; }

	ImGui::SeparatorText(k_addComponentEventMenuLabel.data());

	const auto& l_componentEventRegistry = Utility::StringValueBidirectionalRegistry<Enum::ComponentEvent>::GetInstance();
	const auto& l_executionConditionList = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList           ();

	// Popup内部で利用可能な領域いっぱいまでListBoxを広げる
	if (ImGui::BeginListBox(Constant::k_factoryCheckBoxListLabel.data(), k_addComponentListSize))
	{
		bool l_hasAddableComponentEvent = false;

		for (const auto& [l_key, l_value] : l_componentEventRegistry.GetREFStringToValueMap())
		{
			// Invalidは「未設定」を表す値なので
			// 実際のComponentEventNodeとしては追加しない
			if (l_value == Enum::ComponentEvent::Invalid) { continue; }
	
			// すでに同じComponentEventNodeが存在しているなら
			// 追加候補へ表示しない
			if (std::ranges::any_of(l_executionConditionList,
				                    [l_value](const auto& a_executionCondition)
									{
										return a_executionCondition.m_receiveComponentEvent == l_value;
									}))
			{
				continue;
			}
	
			l_hasAddableComponentEvent = true;
	
			if (!ImGui::Selectable(l_key.c_str())) { continue; }
	
			// Node生成に成功した場合は、
			// 選択官僚としてPopupを閉じる
			if (AddComponentEventNode(m_createNodeScreenSpacePosition, l_value, a_inputComponent))
			{
				ImGui::CloseCurrentPopup();
			}

			// AddComponentEventNode()によってvector画変更されるので
			// iterator/referenceの無効化を避けるため
			// このフレームではそれ以上捜査しない
			break;
		}
	
		// 全ComponentEventが既にt追加済みなら
		// 空Menuにせず理由を表示すっる
		if (!l_hasAddableComponentEvent)
		{
			ImGui::TextDisabled("%s", k_noAddableComponentEventLabel.data());
		}
	
		ImGui::EndListBox();
	}

	ImGui::EndPopup();
}

void FWK::InputComponentInspector::DrawNodeContextMenu(InputComponent& a_inputComponent)
{
	if (!ImGui::BeginPopup(k_nodeContextMenuLabel.data())) { return; }

	// StartはInputComponentの固定入り口なので削除させない
	if (m_contextMenuTargetNodeID == m_startNode.GetVALNodeID()) 
	{
		ImGui::MenuItem(k_startNodeDeleteDisabledLabel.data(), 
			            nullptr,
			            false,
			            false);

		ImGui::EndPopup();

		return;
	}

	if (ImGui::MenuItem(k_deleteNodeMenuLabel.data()))
	{
		RemoveExecutionConditionNode(m_contextMenuTargetNodeID, a_inputComponent);

		m_contextMenuTargetNodeID = Constant::k_invalidNodeEditorID;
	}

	ImGui::EndPopup();
}

bool FWK::InputComponentInspector::PrepareExecutionConditionNode(Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition)
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();
	auto& l_node                = a_executionCondition.m_editorNodeEditor;

	// Deserialize前の古いデータなど
	// NodeIDを持っていないConditionにも対応する
	if (!l_node.FetchVALIsCreated())
	{
		if (!l_node.ApplyNodeID(l_nodeEditorAllocator)) { return false; }
	}

	// ComponentEvent条件Nodeは
	// 左 : 前のNodeから受け取るInput
	// 右 : 次のNodeへ渡すOutput
	// をそれぞれ一個持つ
	const auto& l_inputPinIDList = l_node.GetREFInputPinIDList();

	if (l_inputPinIDList.empty())
	{
		if (!l_node.AddInputPinID(l_nodeEditorAllocator))
		{
			l_node.Release(l_nodeEditorAllocator);

			return false;
		}
	}

	const auto& l_outputPinIDList = l_node.GetREFOutputPinIDList();

	if (l_outputPinIDList.empty())
	{
		if (!l_node.AddOutputPinID(l_nodeEditorAllocator))
		{
			l_node.Release(l_nodeEditorAllocator);

			return false;
		}
	}

	return true;
}

bool FWK::InputComponentInspector::AddComponentEventNode(const ImVec2& a_screeSpacePosition, const Enum::ComponentEvent a_componentEvent, InputComponent& a_inputComponent)
{
	// Invalidは実際のEventNodeとして追加しない
	if (a_componentEvent == Enum::ComponentEvent::Invalid) { return false; }

	auto& l_executionConditionList = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();

	// ContextMenu側でも除外しているが
	// 関数単位でも重複追加を禁止する
	if (std::ranges::any_of(l_executionConditionList,
		                    [a_componentEvent](const auto& a_executionCondition)
		                    {
		                    	return a_executionCondition.m_receiveComponentEvent == a_componentEvent;
		                    }))
	{
		return false;
	}

	// Vector内部でへ直接新しConditionを生成する
	// 一時オブジェクトを作ってNode情報ごとコピーする必要はない
	auto& l_executionCondition = l_executionConditionList.emplace_back();

	l_executionCondition.m_receiveComponentEvent  = a_componentEvent;
	l_executionCondition.m_checkEventLane         = Enum::EventLane::Invalid;
	l_executionCondition.m_expectedObserverResult = false;

	if (!PrepareExecutionConditionNode(l_executionCondition))
	{
		l_executionConditionList.pop_back();

		return false;
	}

	// ContextMenuを開いた瞬間のマウス位置へ
	// 新しいNodeを配置する
	ImNodes::SetNodeScreenSpacePos(l_executionCondition.m_editorNodeEditor.GetVALNodeID(), a_screeSpacePosition);

	return true;
}

void FWK::InputComponentInspector::RemoveExecutionConditionNode(const TypeAlias::NodeEditorID a_nodeID, InputComponent& a_inputComponent)
{
	      auto& l_executionConditionList = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();
	const auto& l_itr                    = std::ranges::find_if(l_executionConditionList, 
		                                                        [a_nodeID](const auto& a_executionCondition)
		                                                        {
																	return a_executionCondition.m_editorNodeEditor.GetVALNodeID() == a_nodeID;
		                                                        });

	// Startや存在しないNodeIDなら何もしない
	if (l_itr == l_executionConditionList.end()) { return; }

	// Vectorから削除する前に
	// Node / InputPin / OutputPinのIDをAllocatorへ返す
	l_itr->m_editorNodeEditor.Release(m_nodeEditor.GetMutableREFNodeEditorAllocator());
	l_executionConditionList.erase   (l_itr);
}