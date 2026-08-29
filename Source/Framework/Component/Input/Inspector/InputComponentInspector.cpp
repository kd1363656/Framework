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

void FWK::InputComponentInspector::PostDeserialize(InputComponent& a_inputComponent)
{
	FWK_ASSERT_RETURN_IF(!SynchronizeNodeGraph(a_inputComponent), "InputComponentのNodeGraphをPostDeserializeで復元できませんでした。");

	// NodeEditorNodeへ保存されているGridSpace座標を
	// ImNodes側のEditorContextへ反映する
	ApplyNodePositions(a_inputComponent);
}

void FWK::InputComponentInspector::EditInspector(InputComponent& a_inputComponent)
{
	if (!m_nodeEditor.BeginDraw()) { return; }

	// 前フレームのPopupで作成されたConditionがある場合、
	// Nodeを描画すより前にDrop位置をImNodesへ反映する
	ApplyPendingConditionNodePosition(a_inputComponent);

	DrawStartNode        ();
	DrawConditionNode    ();
	DrawConditionNodeList(a_inputComponent);
	DrawExecuteNode      (a_inputComponent);

	m_nodeEditor.DrawLinkList();
	m_nodeEditor.EndDraw     ();

	// ユーザーがNodeをドラッグした後の最新座標を
	// NodeEditorNodeのメンバへ戻す
	SynchronizeNodePositionList(a_inputComponent);

	// ImNodesのリンク捜査結果はEndNodeEditor()後に取得する必要がある
	// そのためNodeEditorの描画が完了してから
	// Condition親ノードのOutputPinからリンクが空白へドロップされたか確認する
	RequestConditionNodeCreatePopup();

	// Linkを話した位置へComponentEvent選択Popupを表示する
	DrawConditionNodeCreatePopup(a_inputComponent);
}

nlohmann::json FWK::InputComponentInspector::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::InputComponentInspector::SynchronizeNodeGraph(InputComponent& a_inputComponent)
{
	// 固定で存在する上位MNodeを先に生成する
	if (!SynchronizeStartNode())     { return false; }
	if (!SynchronizeConditionNode()) { return false; }
	if (!SynchronizeExecuteNode())   { return false; }

	// Conditionノードより後のノードを生成する
	if (!SynchronizeConditionNodeList(a_inputComponent)) { return false; }

	SynchronizeLinkList(a_inputComponent);

	return true;
}
bool FWK::InputComponentInspector::SynchronizeStartNode()
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();

	// Startは謬六を受け取るNodeではないため
	// InputPinを持たせない
	if (const auto& l_inputPinIDList = m_startNodeEditorNode.GetREFInputPinIDList();
		!l_inputPinIDList.empty())
	{
		m_startNodeEditorNode.ReleaseInputPinIDList(l_nodeEditorAllocator);
	}

	// NodeIDがまだ存在しない場合だけ生成する
	if (!m_startNodeEditorNode.FetchVALIsCreated())
	{
		if (!m_startNodeEditorNode.ApplyNodeID(l_nodeEditorAllocator)) { return false; }

		// 新しく作ったStartだけ初期座標を設定する
		// DeserializeされたStartには保存座標が既に存在するため
		// ここでは上書きされない
		m_startNodeEditorNode.SetNodePosition(ImVec2{});
	}

	// StartはoutputPinを1個だけ持つ
	if (const auto& l_outputPinIDList = m_startNodeEditorNode.GetREFOutputPinIDList();
		l_outputPinIDList.size() != k_primaryPinCount)
	{
		m_startNodeEditorNode.ReleaseOutputPinIDList(l_nodeEditorAllocator);

		if (!m_startNodeEditorNode.AddOutputPinID(l_nodeEditorAllocator)) { return false; }
	}

	return true;
}
bool FWK::InputComponentInspector::SynchronizeConditionNode()
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();

	// Condition[0]などを束ねる親Conditionノード
	if (!m_rootConditionNodeEditorNode.FetchVALIsCreated())
	{
		if (!m_rootConditionNodeEditorNode.ApplyNodeID(l_nodeEditorAllocator))
		{
			return false;
		}

		m_rootConditionNodeEditorNode.SetNodePosition(ImVec2{ k_initialRootConditionNodePositionX, k_initialRootConditionNodePositionY });
	}

	// Start -> Condition
	// の接続を受け取るInputPinを1個持たせる
	if (const auto& l_inputPinIDList = m_rootConditionNodeEditorNode.GetREFInputPinIDList();
		l_inputPinIDList.size() != k_primaryPinCount)
	{
		m_rootConditionNodeEditorNode.ReleaseInputPinIDList(l_nodeEditorAllocator);

		if (!m_rootConditionNodeEditorNode.AddInputPinID(l_nodeEditorAllocator))
		{
			return false;
		}
	}

	// Condition -> Condition[0]
	// Condition -> Condition[1]
	// の接続元となるOutputPinを1個持たせる
	if (const auto& l_outputPinIDList = m_rootConditionNodeEditorNode.GetREFOutputPinIDList();
		l_outputPinIDList.size() != k_primaryPinCount)
	{
		m_rootConditionNodeEditorNode.ReleaseOutputPinIDList(l_nodeEditorAllocator);

		if (!m_rootConditionNodeEditorNode.AddOutputPinID(l_nodeEditorAllocator))
		{
			return false;
		}
	}

	return true;
}
bool FWK::InputComponentInspector::SynchronizeExecuteNode()
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();

	// Executeは終端ノードなので
	// OutputPinは持たない
	if (const auto& l_outputPinIDList = m_executeNodeEditorNode.GetREFOutputPinIDList();
		!l_outputPinIDList.empty())
	{
		m_executeNodeEditorNode.ReleaseOutputPinIDList(l_nodeEditorAllocator);
	}

	// Execute自身のNodeIDをAllocatorから発行する
	if (!m_executeNodeEditorNode.FetchVALIsCreated())
	{
		if (!m_executeNodeEditorNode.ApplyNodeID(l_nodeEditorAllocator)) { return false; }

		m_executeNodeEditorNode.SetNodePosition(ImVec2{ k_initialExecuteNodePositionX, k_initialExecuteNodePositionY });
	}

	// Start -> Execute
	// の接続を受け取るInputPinを1個持つ
	if (const auto& l_inputPinIDList = m_executeNodeEditorNode.GetREFInputPinIDList();
		l_inputPinIDList.size() != k_primaryPinCount)
	{
		m_executeNodeEditorNode.ReleaseInputPinIDList(l_nodeEditorAllocator);

		if (!m_executeNodeEditorNode.AddInputPinID(l_nodeEditorAllocator))
		{
			return false;
		}
	}

	return true;
}
bool FWK::InputComponentInspector::SynchronizeConditionNodeList(InputComponent& a_inputComponent)
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator                           ();
	auto& l_conditionList       = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();
	
	for (std::size_t l_conditionIndex = 0ULL; l_conditionIndex < l_conditionList.size(); ++l_conditionIndex)
	{
		auto& l_condition           = l_conditionList[l_conditionIndex];
		auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		// Condition[1]は現在終端ノードなので
		// OutputPinを持たない
		if (const auto& l_outputPinIDList = l_conditionNodeEditor.GetREFOutputPinIDList();
			!l_outputPinIDList.empty())
		{
			l_conditionNodeEditor.ReleaseOutputPinIDList(l_nodeEditorAllocator);
		}

		// ConditionにNodeIDがまだ割り当てられていない場合だけ
		// 新しくNodeIDを生成する
		if (!l_conditionNodeEditor.FetchVALIsCreated())
		{
			if (!l_conditionNodeEditor.ApplyNodeID(l_nodeEditorAllocator)) { return false; }

			const float l_conditionPositionY = k_initialChildConditionNodePositionY + k_childConditionNodeVerticalInterval * static_cast<float>(l_conditionIndex);

			l_conditionNodeEditor.SetNodePosition(ImVec2{ k_initialChildConditionNodePositionX, l_conditionPositionY });
		}

		// 親Conditionかｒ接続されるInputPinを1個持つ
		if (const auto& l_inputPinIDList = l_conditionNodeEditor.GetREFInputPinIDList();
			l_inputPinIDList.size() != k_primaryPinCount)
		{
			l_conditionNodeEditor.ReleaseInputPinIDList(l_nodeEditorAllocator);

			if (!l_conditionNodeEditor.AddInputPinID(l_nodeEditorAllocator)) { return false; }
		}	
	}

	return true;
}
void FWK::InputComponentInspector::SynchronizeLinkList(const InputComponent& a_inputComponent)
{
	const auto& l_startOutputPinIDList     = m_startNodeEditorNode.GetREFOutputPinIDList        ();
	const auto& l_conditionInputPinIDList  = m_rootConditionNodeEditorNode.GetREFInputPinIDList ();
	const auto& l_conditionOutputPinIDList = m_rootConditionNodeEditorNode.GetREFOutputPinIDList();
	const auto& l_executeInputPinIDList    = m_executeNodeEditorNode.GetREFInputPinIDList       ();

	// 固定NodeのPinが揃っていなければ
	// Graphを構築できない
	if (l_startOutputPinIDList.empty()      ||
		l_conditionInputPinIDList.empty()   ||
		l_conditionOutputPinIDList.empty() ||
		l_executeInputPinIDList.empty())
	{
		return;
	}

	const auto l_startOutputPinID     = l_startOutputPinIDList    [k_primaryPinIndex];
	const auto l_conditionInputPinID  = l_conditionInputPinIDList [k_primaryPinIndex];
	const auto l_conditionOutputPinID = l_conditionOutputPinIDList[k_primaryPinIndex];
	const auto l_executeInputPinID    = l_executeInputPinIDList   [k_primaryPinIndex];

	// 不要になったLinkを削除
	const auto& l_linkDataList = m_nodeEditor.GetREFLinkDataList();
	      auto  l_linkIndex    = k_initialLinkIndex;

	// RemoveLink()するとvector内部の要素が移動するため
	// 削除時に同じIndexを再確認できるwhileを使用する
	while (l_linkIndex < l_linkDataList.size())
	{
		const auto& l_linkData = l_linkDataList[l_linkIndex];

		// リンクすることが許されているかどうかを確認し、許されているなら
		// インデックスをインクリメントして後続の処理を飛ばす
		if (FetchVALIsAllowedLink(a_inputComponent, l_linkData.m_inputPinID, l_linkData.m_outputPinID))
		{
			++l_linkIndex;

			continue;
		}

		const auto l_removeLinkID = l_linkData.m_linkID;

		m_nodeEditor.RemoveLink(l_removeLinkID);
	}

	// Start -> Condition
	if (!m_nodeEditor.FetchVALHasLink(l_conditionInputPinID, l_startOutputPinID))
	{
		m_nodeEditor.AddLink(l_conditionInputPinID, l_startOutputPinID);
	}

	// Start -> Execute
	if (!m_nodeEditor.FetchVALHasLink(l_executeInputPinID, l_startOutputPinID))
	{
		m_nodeEditor.AddLink(l_executeInputPinID, l_startOutputPinID);
	}

	// Condition -> Condition[i]
	const auto& l_conditionList  = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();
	      
	for (const auto& l_condition : l_conditionList)
	{
		const auto& l_childConditionInputPinIDList = l_condition.m_editorNodeEditor.GetREFInputPinIDList();

		if (l_childConditionInputPinIDList.empty()) { continue; }

		const auto l_childConditionInputPinID = l_childConditionInputPinIDList[k_primaryPinIndex];

		if (m_nodeEditor.FetchVALHasLink(l_childConditionInputPinID, l_conditionOutputPinID)) { continue; }

		m_nodeEditor.AddLink(l_childConditionInputPinID, l_conditionOutputPinID);
	}
}
void FWK::InputComponentInspector::SynchronizeNodePositionList(InputComponent& a_inputComponent)
{
	// Startノード
	if (m_startNodeEditorNode.FetchVALIsCreated())
	{
		m_startNodeEditorNode.SetNodePosition(ImNodes::GetNodeGridSpacePos(m_startNodeEditorNode.GetVALNodeID()));
	}

	// Conditionルートノード
	if (m_rootConditionNodeEditorNode.FetchVALIsCreated())
	{
		m_rootConditionNodeEditorNode.SetNodePosition(ImNodes::GetNodeGridSpacePos(m_rootConditionNodeEditorNode.GetVALNodeID()));
	}

	// Executeノード
	if (m_executeNodeEditorNode.FetchVALIsCreated())
	{
		m_executeNodeEditorNode.SetNodePosition(ImNodes::GetNodeGridSpacePos(m_executeNodeEditorNode.GetVALNodeID()));
	}

	// Conditionルートノードの子ノード
	auto& l_conditionList  = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();
	
	for (auto& l_condition : l_conditionList)
	{
		auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		if (!l_conditionNodeEditor.FetchVALIsCreated()) { continue; }

		l_conditionNodeEditor.SetNodePosition(ImNodes::GetNodeGridSpacePos(l_conditionNodeEditor.GetVALNodeID()));
	}
}

void FWK::InputComponentInspector::ApplyNodePositions(const InputComponent& a_inputComponent)
{
	// Startノードの位置をエディターに反映
	if (m_startNodeEditorNode.FetchVALIsCreated())
	{
		FWK_ASSERT_RETURN_IF(!m_nodeEditor.ApplyNodePosition(m_startNodeEditorNode), "Startノードの座標反映に失敗しました。");
	}

	// Conditionノードの位置をエディターに反映
	if (m_rootConditionNodeEditorNode.FetchVALIsCreated())
	{
		FWK_ASSERT_RETURN_IF(!m_nodeEditor.ApplyNodePosition(m_rootConditionNodeEditorNode), "Conditionノードの座標反映に失敗しました。");
	}

	// Executeノードの位置をエディターに反映
	if (m_executeNodeEditorNode.FetchVALIsCreated())
	{
		FWK_ASSERT_RETURN_IF(!m_nodeEditor.ApplyNodePosition(m_executeNodeEditorNode), "Executeノードの座標反映に失敗しました。");
	}

	const auto& l_conditionList  = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();
	     
	for (const auto& l_condition : l_conditionList)
	{
		const auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		if (!l_conditionNodeEditor.FetchVALIsCreated())
		{
			continue;
		}

		FWK_ASSERT_RETURN_IF(!m_nodeEditor.ApplyNodePosition(l_conditionNodeEditor), "Condition子ノードの座標反映に失敗しました。");
	}
}
void FWK::InputComponentInspector::ApplyPendingConditionNodePosition(InputComponent& a_inputComponent)
{
	// 配置待ちNodeが存在しなければ何もしない
	if (m_pendingConditionNodePositionNodeID == Constant::k_invalidNodeEditorID) { return; }

	auto& l_conditionList = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();

	for (auto& l_condition : l_conditionList)
	{
		auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		if (l_conditionNodeEditor.GetVALNodeID() != m_pendingConditionNodePositionNodeID) { continue; }

		ImNodes::SetNodeScreenSpacePos(l_conditionNodeEditor.GetVALNodeID(), m_pendingConditionNodeCreateScreenPosition);

		// ImNodesへ設定した位置をGridSpaceとして取り直す
		l_conditionNodeEditor.SetNodePosition(ImNodes::GetNodeGridSpacePos(l_conditionNodeEditor.GetVALNodeID()));

		m_pendingConditionNodePositionNodeID       = Constant::k_invalidNodeEditorID;
		m_pendingConditionNodeCreateScreenPosition = {};

		break;
	}

	// 一回反映したら予約を解除
	m_pendingConditionNodePositionNodeID       = Constant::k_invalidNodeEditorID;
	m_pendingConditionNodeCreateScreenPosition = {};
}

void FWK::InputComponentInspector::DrawStartNode() const
{
	const auto& l_outputPinIDList = m_startNodeEditorNode.GetREFOutputPinIDList();

	if (l_outputPinIDList.empty()) { return; }

	ImNodes::BeginNode(m_startNodeEditorNode.GetVALNodeID());

	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted    (k_startNodeLabel.data());
	ImNodes::EndNodeTitleBar  ();

	ImNodes::BeginOutputAttribute(l_outputPinIDList[k_primaryPinIndex]);
	ImGui::TextUnformatted       (k_outputPinLabel.data());
	ImNodes::EndOutputAttribute  ();
	ImNodes::EndNode             ();
}
void FWK::InputComponentInspector::DrawConditionNode() const
{
	const auto& l_inputPinIDList  = m_rootConditionNodeEditorNode.GetREFInputPinIDList();
	const auto& l_outputPinIDList = m_rootConditionNodeEditorNode.GetREFOutputPinIDList();

	if (l_inputPinIDList.empty() ||
		l_outputPinIDList.empty())
	{
		return;
	}

	ImNodes::BeginNode(m_rootConditionNodeEditorNode.GetVALNodeID());
	ImGui::PushID     (m_rootConditionNodeEditorNode.GetVALNodeID());

	// タイトルバーの描画
	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted    (k_conditionNodeLabel.data());
	ImNodes::EndNodeTitleBar  ();

	// Inputピンの描画
	ImNodes::BeginInputAttribute(l_inputPinIDList[k_primaryPinIndex]);
	ImGui::TextUnformatted      (k_inputPinLabel.data());
	ImNodes::EndInputAttribute  ();

	// Outputピンの描画
	ImNodes::BeginOutputAttribute(l_outputPinIDList[k_primaryPinIndex]);
	ImGui::TextUnformatted       (k_outputPinLabel.data());
	ImNodes::EndOutputAttribute  ();

	ImGui::PopID    ();
	ImNodes::EndNode();
}
void FWK::InputComponentInspector::DrawConditionNodeList(InputComponent& a_inputComponent) const
{
	auto& l_conditionList  = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();
	
	for (std::size_t l_conditionIndex = 0ULL; l_conditionIndex < l_conditionList.size(); ++l_conditionIndex)
	{
			  auto& l_condition           = l_conditionList[l_conditionIndex];
		const auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;
		const auto& l_inputPinIDList      = l_conditionNodeEditor.GetREFInputPinIDList();

		if (l_inputPinIDList.empty()) { continue; }

		ImNodes::BeginNode(l_conditionNodeEditor.GetVALNodeID());
		ImGui::PushID     (std::addressof(l_condition));

		const auto& l_conditionNodeText = std::format("{}[{}]", k_conditionNodeLabel.data(), std::to_string(l_conditionIndex));

		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted    (l_conditionNodeText.c_str());
		ImNodes::EndNodeTitleBar  ();

		// Condition[i]は親Conditionから入力されるのでInputPinだけを持つ
		ImNodes::BeginInputAttribute(l_inputPinIDList[k_primaryPinIndex]);
		ImGui::TextUnformatted       (k_inputPinLabel.data());
		ImNodes::EndInputAttribute   ();

		Utility::StringValueBidirectionalRegistryRadioButtonSelector(k_notifyComponentEventLabel,          l_condition.m_receiveComponentEvent);
		Utility::StringValueBidirectionalRegistryRadioButtonSelector(k_notifyEventLaneLabel,               l_condition.m_checkEventLane);
		ImGui::Checkbox                                             (k_expectedObserverResultLabel.data(), &l_condition.m_expectedObserverResult);
		ImGui::PopID                                                ();

		ImNodes::EndNode();
	}
}
void FWK::InputComponentInspector::DrawConditionNodeCreatePopup(InputComponent& a_inputComponent)
{
	if (!ImGui::IsPopupOpen(k_conditionNodeCreatePopupLabel.data())) { return; }

	// Linkを話した位置へPopupを表示する
	ImGui::SetNextWindowPos(m_conditionNodeDropScreenPosition, ImGuiCond_Appearing);

	if (!ImGui::BeginPopup(k_conditionNodeCreatePopupLabel.data())) { return; }

	// 今回OpenPopup()を呼ぶのはLinkをDropした瞬間なのでこの座標 = Linkを話した座標になる
	const auto& l_nodeCreateScreenPosition = ImGui::GetMousePosOnOpeningCurrentPopup();

	ImGui::TextUnformatted(k_notifyComponentEventLabel.data());

	if (!ImGui::BeginListBox(k_conditionNodeCreateListBoxLabel.data())) 
	{
		ImGui::EndPopup();

		return; 
	}
	
	const auto& l_componentEventRegistry = Utility::StringValueBidirectionalRegistry<Enum::ComponentEvent>::GetInstance();
	const auto& l_componentEventMap      = l_componentEventRegistry.GetREFStringToValueMap                             ();

	bool l_hasUnusedComponentEvent = false;

	for (const auto& [l_componentEventName, l_componentEvent] : l_componentEventMap)
	{
		// Invalidは実際のConditionとして作成しない
		if (l_componentEvent == Enum::ComponentEvent::Invalid) { continue; }

		// 既に別Conditionで使用されているイベントは表示しない
		if (FetchVALIsComponentEventUsed(a_inputComponent, l_componentEvent)) { continue; }

		l_hasUnusedComponentEvent = true;

		if (!ImGui::Selectable(l_componentEventName.c_str())) { continue; }

		if (AddConditionNode(l_componentEvent, l_nodeCreateScreenPosition, a_inputComponent))
		{
			ImGui::CloseCurrentPopup();
		}

		break;
	}

	if (!l_hasUnusedComponentEvent)
	{
		ImGui::TextDisabled("%s", k_noUnusedComponentEventLabel.data());
	}

	ImGui::EndListBox();
	ImGui::EndPopup  ();
}
void FWK::InputComponentInspector::DrawExecuteNode(InputComponent& a_inputComponent) const
{
	      auto& l_execution      = a_inputComponent.GetMutableREFExecution     ();
		  auto& l_notifyStrategy = a_inputComponent.GetMutableREFNotifyStrategy();
	const auto& l_inputPinIDList = m_executeNodeEditorNode.GetREFInputPinIDList();

	if (l_inputPinIDList.empty()) { return; }

	ImNodes::BeginNode(m_executeNodeEditorNode.GetVALNodeID());
	ImGui::PushID     (m_executeNodeEditorNode.GetVALNodeID());

	// タイトルバーの描画
	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted    (k_executeNodeLabel.data());
	ImNodes::EndNodeTitleBar  ();

	// Inputピンの描画
	ImNodes::BeginInputAttribute(l_inputPinIDList[k_primaryPinIndex]);
	ImGui::TextUnformatted      (k_inputPinLabel.data());
	ImNodes::EndInputAttribute  ();

	// セレクターからストラテジーが生成されたなら適用
	Utility::FactoryRadioButtonSelector<TypeAlias::ComponentEventNotifyStrategyUniqueFactory>(k_componentEventNotifyStrategyRadioButtonSelectorLabel, l_notifyStrategy);
	Utility::StringValueBidirectionalRegistryRadioButtonSelector                             (k_notifyComponentEventLabel,                            l_execution.m_notifyComponentEvent);
	Utility::StringValueBidirectionalRegistryRadioButtonSelector                             (k_notifyEventLaneLabel,                                 l_execution.m_notifyEventLane);
	ImGui::Checkbox                                                                          (k_notifyFlagLabel.data(),                               &l_execution.m_notifyFlag);

	ImGui::PopID    ();
	ImNodes::EndNode();
}

void FWK::InputComponentInspector::RequestConditionNodeCreatePopup()
{
	const auto& l_rootConditionOutputPinIDList = m_rootConditionNodeEditorNode.GetREFOutputPinIDList();

	if (l_rootConditionOutputPinIDList.empty()) { return; }

	auto l_startedPinID = Constant::k_invalidNodeEditorID;

	// 第二引数false
	// 既存Linkを切り離してDropした場合は対象にせず、
	// 新しくPinから延ばしたLinkを空白へDrop下場合だけ検出する
	if (!ImNodes::IsLinkDropped(std::addressof(l_startedPinID), false)) { return; }

	// RootConditionのOutputPin以外から延ばされたLinkでは
	// Condition作成Popupを開かない
	if (l_startedPinID != l_rootConditionOutputPinIDList[k_primaryPinIndex]) { return; }

	// Popupと新規Conditionノードの両方で使用するため、
	// Linkを話した瞬間のScreenSpace座標を保存する
	m_conditionNodeDropScreenPosition = ImGui::GetMousePos();

	ImGui::OpenPopup(k_conditionNodeCreatePopupLabel.data());
}

bool FWK::InputComponentInspector::AddConditionNode(const Enum::ComponentEvent a_componentEvent, const ImVec2 & a_nodeScreenPosition, InputComponent & a_inputComponent)
{
	// InvalidはConditionとして生成しない
	if (a_componentEvent == Enum::ComponentEvent::Invalid) { return false; }

	// Popup以外からこの間数が呼ばれるようになった場合にも
	// ComponentEventの重複を防げるように検証
	if (FetchVALIsComponentEventUsed(a_inputComponent, a_componentEvent)) { return false; }

	const auto& l_rootConditionOutputPinIDList = m_rootConditionNodeEditorNode.GetREFOutputPinIDList();

	// rootConditionのoutputPinが存在しなければ
	// Condition[i]を接続できない
	if (l_rootConditionOutputPinIDList.empty()) { return false; }

	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator                           ();
	auto& l_conditionList       = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();

	// Conditionデータを追加する
	l_conditionList.emplace_back();

	auto& l_condition           = l_conditionList.back();
	auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

	l_condition.m_receiveComponentEvent = a_componentEvent;

	// Condition[i]のNodeIDを発行する
	if (!l_conditionNodeEditor.ApplyNodeID(l_nodeEditorAllocator))
	{
		l_conditionList.pop_back();

		return false;
	}

	// RootCondition -> Condition[i]の接続を受け取るInputPinを発行する
	if (!l_conditionNodeEditor.AddInputPinID(l_nodeEditorAllocator))
	{
		l_conditionNodeEditor.Release(l_nodeEditorAllocator);
		l_conditionList.pop_back     ();

		return false;
	}

	const auto& l_conditionInputPinIDList  = l_conditionNodeEditor.GetREFInputPinIDList();
	const auto  l_conditionInputPinID      = l_conditionInputPinIDList     [k_primaryPinIndex];
	const auto  l_rootConditionOutputPinID = l_rootConditionOutputPinIDList[k_primaryPinIndex];

	if (!m_nodeEditor.AddLink(l_conditionInputPinID, l_rootConditionOutputPinID))
	{
		l_conditionNodeEditor.Release(l_nodeEditorAllocator);
		l_conditionList.pop_back     ();

		return false;
	}

	// 次FrameのBeginNodeEditor(9直後に配置するため、
	// NodeIDと「Popupを開いた瞬間の座標」を予約する
	m_pendingConditionNodePositionNodeID       = l_conditionNodeEditor.GetVALNodeID();
	m_pendingConditionNodeCreateScreenPosition = a_nodeScreenPosition;
	
	return true;
}

bool FWK::InputComponentInspector::FetchVALIsAllowedLink(const InputComponent& a_inputComponent, const TypeAlias::NodeEditorID a_inputPinID, const TypeAlias::NodeEditorID a_outputPinID) const
{
	const auto& l_startOutputPinIDList     = m_startNodeEditorNode.GetREFOutputPinIDList        ();
	const auto& l_conditionInputPinIDList  = m_rootConditionNodeEditorNode.GetREFInputPinIDList ();
	const auto& l_conditionOutputPinIDList = m_rootConditionNodeEditorNode.GetREFOutputPinIDList();
	const auto& l_executeInputPinIDList    = m_executeNodeEditorNode.GetREFInputPinIDList       ();

	// Start-> Condition
	// Start-> Execute
	if (!l_startOutputPinIDList.empty() &&
		a_outputPinID == l_startOutputPinIDList[k_primaryPinIndex])
	{
		if (!l_conditionInputPinIDList.empty() &&
			a_inputPinID == l_conditionInputPinIDList[k_primaryPinIndex])
		{
			return true;
		}

		if (!l_executeInputPinIDList.empty() &&
			a_inputPinID == l_executeInputPinIDList[k_primaryPinIndex])
		{
			return true;
		}

		return false;
	}

	// Condition -> Condition[i]
	if (l_conditionOutputPinIDList.empty() ||
		a_outputPinID != l_conditionOutputPinIDList[k_primaryPinIndex])
	{
		return false;
	}

	const auto& l_conditionList  = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();
	      
	for (const auto& l_condition : l_conditionList)
	{
		if (const auto& l_childConditionInputPinIDList = l_condition.m_editorNodeEditor.GetREFInputPinIDList();
			!l_childConditionInputPinIDList.empty() &&
			a_inputPinID == l_childConditionInputPinIDList[k_primaryPinIndex])
		{
			return true;
		}
	}

	return false;
}

bool FWK::InputComponentInspector::FetchVALIsComponentEventUsed(const InputComponent& a_inputComponent, const Enum::ComponentEvent a_componentEvent) const
{
	const auto& l_conditionList = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();

	return std::any_of(l_conditionList.begin(), l_conditionList.end(), [a_componentEvent](const auto& a_condition)
					  {
							return a_condition.m_receiveComponentEvent == a_componentEvent;
					  });
}