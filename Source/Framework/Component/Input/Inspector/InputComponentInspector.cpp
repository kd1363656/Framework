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
	if (!m_nodeEditor.BeginDraw()) { return; }

	// Node/Pin/Linkが不足していた場合は
	// 現在のInputComponentの固定構造に合わせて生成する
	if (!SynchronizeNodeGraph(a_inputComponent))
	{
		m_nodeEditor.EndDraw();

		return;
	}

	// 保存されているNode座標をImNodesへ反映する
	// 毎フレーム実行するとドラッグ操作を上書きするため
	// 最初の一フレームだけ実行
	if (!m_isDefaultNodePositionApplied)
	{
		ApplyDefaultNodePositions(a_inputComponent);

		m_isDefaultNodePositionApplied = true;
	}

	DrawStartNode        ();
	DrawConditionNodeList(a_inputComponent);
	DrawExecuteNode      (a_inputComponent);

	m_nodeEditor.DrawLinkList();
	m_nodeEditor.EndDraw     ();

	// ユーザーがNodeをドラッグした後の最新座標を
	// NodeEditorNodeのメンバへ戻す
	SynchronizeNodePositionList(a_inputComponent);
}

nlohmann::json FWK::InputComponentInspector::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::InputComponentInspector::SynchronizeNodeGraph(InputComponent& a_inputComponent)
{
	if (!SynchronizeStartNode())                         { return false; }
	if (!SynchronizeConditionNodeList(a_inputComponent)) { return false; }
	if (!SynchronizeExecuteNode(a_inputComponent))       { return false; }

	SynchronizeLinkList(a_inputComponent);

	return true;
}
bool FWK::InputComponentInspector::SynchronizeStartNode()
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();

	// Startは謬六を受け取るNodeではないため
	// InputPinを持たせない
	const auto& l_inputPinIDList = m_startNodeEditor.GetREFInputPinIDList();

	if (!l_inputPinIDList.empty()) 
	{
		m_startNodeEditor.ReleaseInputPinIDList(l_nodeEditorAllocator);
	}

	// NodeIDがまだ存在しない場合だけ生成する
	if (!m_startNodeEditor.FetchVALIsCreated())
	{
		if (!m_startNodeEditor.ApplyNodeID(l_nodeEditorAllocator)) { return false; }

		// 新しく作ったStartだけ初期座標を設定する
		// DeserializeされたStartには保存座標が既に存在するため
		// ここでは上書きされない
		m_startNodeEditor.SetNodePosition(ImVec2{ k_startNodePositionX, k_startNodePositionY });
	}

	// StartはoutputPinを1個だけ持つ
	const auto& l_outputPinIDList = m_startNodeEditor.GetREFOutputPinIDList();

	if (l_outputPinIDList.size() != k_primaryPinCount)
	{
		m_startNodeEditor.ReleaseOutputPinIDList(l_nodeEditorAllocator);

		if (!m_startNodeEditor.AddOutputPinID(l_nodeEditorAllocator)) { return false; }
	}

	return true;
}
bool FWK::InputComponentInspector::SynchronizeExecuteNode(InputComponent& a_inputComponent)
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator();
	auto& l_execution           = a_inputComponent.GetMutableREFExecution      ();
	auto& l_executeNodeEditor   = l_execution.m_editorNodeEditor;

	// Executeは他Nodeへ接続する必要がないため
	// OutputPinを持たない
	if (const auto& l_outputPintIDList = l_executeNodeEditor.GetREFOutputPinIDList();
		!l_outputPintIDList.empty())
	{
		l_executeNodeEditor.ReleaseOutputPinIDList(l_nodeEditorAllocator);
	}

	// ExecuteのNodeIDがまだ存在しない場合だけ生成する
	if (!l_executeNodeEditor.FetchVALIsCreated())
	{
		if (!l_executeNodeEditor.ApplyNodeID(l_nodeEditorAllocator)) { return false; }

		l_executeNodeEditor.SetNodePosition(ImVec2{ k_executeNodePositionX, k_executeNodePositionY });
	}

	// ExecuteはStart空接続されるInputPinを1個だけ持つ
	if (const auto& l_inputPinIDList = l_executeNodeEditor.GetREFInputPinIDList();
		l_inputPinIDList.size() != k_primaryPinCount)
	{
		l_executeNodeEditor.ReleaseInputPinIDList(l_nodeEditorAllocator);

		if (!l_executeNodeEditor.AddInputPinID(l_nodeEditorAllocator)) { return false; }
	}

	return true;
}
bool FWK::InputComponentInspector::SynchronizeConditionNodeList(InputComponent& a_inputComponent)
{
	auto& l_nodeEditorAllocator = m_nodeEditor.GetMutableREFNodeEditorAllocator                           ();
	auto& l_conditionList       = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();

	auto l_conditionIndex = k_initialConditionIndex;

	while (l_conditionIndex < l_conditionList.size())
	{
		auto& l_condition           = l_conditionList[l_conditionIndex];
		auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		// ConditionはStartから接続されるだけなので
		// OutputPinは持たない
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

			const float l_conditionPositionY = k_conditionNodeFirstPositionY + k_conditionNodeVerticalInterval * static_cast<float>(l_conditionIndex);

			l_conditionNodeEditor.SetNodePosition(ImVec2{ k_conditionNodePositionX, l_conditionPositionY });
		}

		// ConditionはStartからLinkされるInputPinを
		// 必ず一個だけ持つ
		if (const auto& l_inputPinIDList = l_conditionNodeEditor.GetREFInputPinIDList();
			l_inputPinIDList.size() != k_primaryPinCount)
		{
			l_conditionNodeEditor.ReleaseInputPinIDList(l_nodeEditorAllocator);

			if (!l_conditionNodeEditor.AddInputPinID(l_nodeEditorAllocator)) { return false; }
		}

		++l_conditionIndex;
	}

	return true;
}
void FWK::InputComponentInspector::SynchronizeLinkList(const InputComponent& a_inputComponent)
{
	const auto& l_startOutputPinIDList = m_startNodeEditor.GetREFOutputPinIDList();

	if (l_startOutputPinIDList.empty()) { return; }

	const auto l_startOutputPinID = l_startOutputPinIDList[k_primaryPinIndex];

	      auto  l_linkIndex    = k_initialLinkIndex;
	const auto& l_linkDataList = m_nodeEditor.GetREFLinkDataList();

	while (l_linkIndex < l_linkDataList.size())
	{
		const auto& l_linkData =l_linkDataList[l_linkIndex];

		// 現在許可されている接続元は
		// StartのOutputPinだけ
		const bool l_isStartOutputPin  = l_linkData.m_outputPinID == l_startOutputPinID;
		const bool l_isAllowedInputPin = FetchVALIsAllowedInputPin(a_inputComponent, l_linkData.m_inputPinID);

		if (l_isStartOutputPin &&
			l_isAllowedInputPin)
		{
			++l_linkIndex;

			continue;
		}

		// RemoveLink(9でvectorが変化する前に
		// LinkIDだけ値で取得しておく
		const auto l_removeLinkID = l_linkData.m_linkID;

		m_nodeEditor.RemoveLink(l_removeLinkID);
	}

	const auto& l_conditionList = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();

	auto l_conditionIndex = k_initialConditionIndex;

	while (l_conditionIndex < l_conditionList.size())
	{
		const auto& l_condition               = l_conditionList[l_conditionIndex];
		const auto& l_conditionInputPinIDList = l_condition.m_editorNodeEditor.GetREFInputPinIDList();

		if (!l_conditionInputPinIDList.empty()) 
		{
			const auto l_conditionInputPinID = l_conditionInputPinIDList[k_primaryPinIndex];

			// Start->このConditionがまだ存在していない場合だけ
			// Linkを新規生成する
			if (!m_nodeEditor.FetchVALHasLink(l_startOutputPinID, l_conditionInputPinID))
			{
				m_nodeEditor.AddLink(l_startOutputPinID, l_conditionInputPinID);
			}
		}

		++l_conditionIndex;
	}

	const auto& l_execution             = a_inputComponent.GetREFExecution                   ();
	const auto& l_executeInputPinIDList = l_execution.m_editorNodeEditor.GetREFInputPinIDList();

	if (l_executeInputPinIDList.empty()) { return; }

	const auto l_executeInputPinID = l_executeInputPinIDList[k_primaryPinIndex];

	if (m_nodeEditor.FetchVALHasLink(l_startOutputPinID, l_executeInputPinID)) { return; }

	m_nodeEditor.AddLink(l_startOutputPinID, l_executeInputPinID);
}
void FWK::InputComponentInspector::SynchronizeNodePositionList(InputComponent& a_inputComponent)
{
	// ImNodes上でユーザーが移動させた現在位置を取得して
	// NodeEditorNode自身が保持する座標へ戻す
	if (m_startNodeEditor.FetchVALIsCreated())
	{
		m_startNodeEditor.SetNodePosition(ImNodes::GetNodeGridSpacePos(m_startNodeEditor.GetVALNodeID()));
	}

	auto& l_conditionList  = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();
	auto  l_conditionIndex = k_initialConditionIndex;

	while (l_conditionIndex < l_conditionList.size())
	{
		auto& l_condition           = l_conditionList[l_conditionIndex];
		auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		if (l_conditionNodeEditor.FetchVALIsCreated())
		{
			l_conditionNodeEditor.SetNodePosition(ImNodes::GetNodeGridSpacePos(l_conditionNodeEditor.GetVALNodeID()));
		}

		++l_conditionIndex;
	}

	auto& l_execution         = a_inputComponent.GetMutableREFExecution();
	auto& l_executeNodeEditor = l_execution.m_editorNodeEditor;

	if (!l_executeNodeEditor.FetchVALIsCreated()) { return; }

	l_executeNodeEditor.SetNodePosition(ImNodes::GetNodeGridSpacePos(l_executeNodeEditor.GetVALNodeID()));
}

void FWK::InputComponentInspector::ApplyDefaultNodePositions(InputComponent& a_inputComponent)
{
	if (m_startNodeEditor.FetchVALIsCreated())
	{
		ImNodes::SetNodeGridSpacePos(m_startNodeEditor.GetVALNodeID(), m_startNodeEditor.GetREFNodePosition());
	}

	auto& l_conditionList = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();

	auto l_conditionIndex = k_initialConditionIndex;

	while (l_conditionIndex < l_conditionList.size())
	{
		auto& l_condition           = l_conditionList[l_conditionIndex];
		auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;

		if (l_conditionNodeEditor.FetchVALIsCreated()) 
		{
			ImNodes::SetNodeGridSpacePos(l_conditionNodeEditor.GetVALNodeID(), l_conditionNodeEditor.GetREFNodePosition());
		}

		++l_conditionIndex;
	}

	auto& l_execution         = a_inputComponent.GetMutableREFExecution();
	auto& l_executeNodeEditor = l_execution.m_editorNodeEditor;

	if (!l_executeNodeEditor.FetchVALIsCreated()) { return; }

	ImNodes::SetNodeGridSpacePos(l_executeNodeEditor.GetVALNodeID(), l_executeNodeEditor.GetREFNodePosition());
}

void FWK::InputComponentInspector::DrawStartNode() const
{
	const auto& l_outputPinIDList = m_startNodeEditor.GetREFOutputPinIDList();

	if (l_outputPinIDList.empty()) { return; }

	ImNodes::BeginNode(m_startNodeEditor.GetVALNodeID());

	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted    (k_startNodeLabel.data());
	ImNodes::EndNodeTitleBar  ();

	ImNodes::BeginOutputAttribute(l_outputPinIDList[k_primaryPinIndex]);
	ImGui::TextUnformatted       (k_outputPinLabel.data());
	ImNodes::EndOutputAttribute  ();
	ImNodes::EndNode             ();
}
void FWK::InputComponentInspector::DrawConditionNodeList(InputComponent& a_inputComponent) const
{
	auto& l_conditionList  = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList();
	auto  l_conditionIndex = k_initialConditionIndex;

	while (l_conditionIndex < l_conditionList.size())
	{
		      auto& l_condition           = l_conditionList[l_conditionIndex];
		      auto& l_conditionNodeEditor = l_condition.m_editorNodeEditor;
		const auto& l_inputPinIDList      = l_conditionNodeEditor.GetREFInputPinIDList();

		if (l_inputPinIDList.empty())
		{
			++l_conditionIndex;
			
			continue;
		}

		ImNodes::BeginNode        (l_conditionNodeEditor.GetVALNodeID());
		ImGui::PushID             (l_conditionNodeEditor.GetVALNodeID());
		ImNodes::BeginNodeTitleBar();
		ImGui::TextUnformatted    (k_conditionNodeLabel.data());
		ImNodes::EndNodeTitleBar  ();

		// ConditionはStartから入力されるだけなので
		// InputPinだけを持つ
		ImNodes::BeginInputAttribute(l_inputPinIDList[k_primaryPinIndex]);
		ImGui::TextUnformatted       (k_inputPinLabel.data());
		ImNodes::EndInputAttribute   ();

		Utility::StringValueBidirectionalRegistryRadioButtonSelector(k_notifyComponentEventLabel, l_condition.m_receiveComponentEvent);
		Utility::StringValueBidirectionalRegistryRadioButtonSelector(k_notifyComponentEventLabel, l_condition.m_checkEventLane);

		ImGui::Checkbox(k_expectedObserverResultLabel.data(), &l_condition.m_expectedObserverResult);
		ImGui::PopID   ();

		ImNodes::EndNode();

		++l_conditionIndex;
	}
}
void FWK::InputComponentInspector::DrawExecuteNode(InputComponent& a_inputComponent) const
{
	      auto& l_execution         = a_inputComponent.GetMutableREFExecution();
	      auto& l_executeNodeEditor = l_execution.m_editorNodeEditor;
	const auto& l_inputPinIDList    = l_executeNodeEditor.GetREFInputPinIDList();

	if (l_inputPinIDList.empty()) { return; }

	ImNodes::BeginNode(l_executeNodeEditor.GetVALNodeID());
	ImGui::PushID     (l_executeNodeEditor.GetVALNodeID());

	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted    (k_executeNodeLabel.data());
	ImNodes::EndNodeTitleBar  ();

	ImNodes::BeginInputAttribute(l_inputPinIDList[k_primaryPinIndex]);
	ImGui::TextUnformatted      (k_inputPinLabel.data());
	ImNodes::EndInputAttribute  ();

	// InputComponentが実際に通知する
	Utility::StringValueBidirectionalRegistryRadioButtonSelector(k_notifyComponentEventLabel, l_execution.m_notifyComponentEvent);
	Utility::StringValueBidirectionalRegistryRadioButtonSelector(k_notifyEventLaneLabel,      l_execution.m_notifyEventLane);

	ImGui::PopID    ();
	ImNodes::EndNode();
}

bool FWK::InputComponentInspector::FetchVALIsAllowedInputPin(const InputComponent& a_inputComponent, const TypeAlias::NodeEditorID a_inputPinID) const
{
	const auto& l_conditionList  = a_inputComponent.GetREFNotifyComponentEventExecutionConditionList();
	      auto  l_conditionIndex = k_initialConditionIndex;

	while (l_conditionIndex < l_conditionList.size())
	{
		const auto& l_condition      = l_conditionList[l_conditionIndex];
		const auto& l_inputPinIDList = l_condition.m_editorNodeEditor.GetREFInputPinIDList();

		if (!l_inputPinIDList.empty() &&
			l_inputPinIDList[k_primaryPinIndex] == a_inputPinID)
		{
			return true;
		}

		++l_conditionIndex;
	}

	const auto& l_execution               = a_inputComponent.GetREFExecution                   ();
	const auto& l_executionInputPinIDList = l_execution.m_editorNodeEditor.GetREFInputPinIDList();

	if (l_executionInputPinIDList.empty()) { return false; }

	return l_executionInputPinIDList[k_primaryPinIndex] == a_inputPinID;
}