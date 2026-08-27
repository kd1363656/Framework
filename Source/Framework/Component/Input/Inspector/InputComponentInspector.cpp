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
	// ノードセレクターだけ先に描画
	DrawExecutionCheckBoxList(a_inputComponent);

	// DetailsEditorWindow内部でNodeEditorが
	// 必要以上に縦へ広がらないよう固定高さのChildを作る
	ImGui::BeginChild(k_nodeEditorLabel.data(), Constant::k_defaultNodeEditorCanvasSize);

	if (m_nodeEditor.BeginDraw())
	{
		m_nodeEditor.EndDraw();
	}

	ImGui::EndChild();
}

nlohmann::json FWK::InputComponentInspector::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::InputComponentInspector::DrawExecutionCheckBoxList(InputComponent& a_inputComponent)
{
	const auto& l_stringValueBidirectionalRegistry           = Utility::StringValueBidirectionalRegistry<Enum::ComponentEvent>::GetInstance();
	      auto& l_notifyComponentEventExecutionConditionList = a_inputComponent.GetMutableREFNotifyComponentEventExecutionConditionList    ();
		  auto& l_nodeEditorAllocator                        = m_nodeEditor.GetMutableREFNodeEditorAllocator                               ();

	ImGui::PushID    (std::addressof(a_inputComponent));
	ImGui::BeginGroup();
	
	ImGui::SeparatorText(k_executionConditionCheckBoxLabel.data());

	// -1.0Fを使用すると
	// 現在利用可能な横幅いっぱいまでリストを広げる
	if (const float l_listHeight = ImGui::GetTextLineHeightWithSpacing() * Constant::k_defaultChildVisibleItemCount;
		!ImGui::BeginListBox(Constant::k_factoryCheckBoxListLabel.data(), ImVec2(Constant::k_childWindowMaxSizeX, l_listHeight)))
	{
		ImGui::EndGroup();
		ImGui::PopID   ();

		return;
	}

	for (const auto& [l_key, l_value] : l_stringValueBidirectionalRegistry.GetREFStringToValueMap())
	{
		bool l_isSelected = false;

		const auto& l_itr = std::ranges::find_if(l_notifyComponentEventExecutionConditionList.begin(), l_notifyComponentEventExecutionConditionList.end(), [l_value](const auto& a_executionCondition)
				                                 {
												 	 return a_executionCondition.m_receiveComponentEvent == l_value;
				                                 });

		if (l_itr != l_notifyComponentEventExecutionConditionList.end())
		{
			// リスト内部に存在するならtrue
			l_isSelected = true;
		}

		// CheckBoxのOn/Offが切り替えられていないならcontinue
		if (!ImGui::Checkbox(l_key.c_str(), &l_isSelected)) { continue; }

		// チェックボックスのチェックが外されるかつけられるかで実行する
		if (l_isSelected) 
		{
			Struct::ObserverInputExecutionCondition<Enum::ComponentEvent> l_executionCondition = {};

			l_executionCondition.m_receiveComponentEvent  = l_value;
			l_executionCondition.m_checkEventLane         = Enum::EventLane::Invalid;
			l_executionCondition.m_expectedObserverResult = false;
			
			auto& l_editorNodeEditor = l_executionCondition.m_editorNodeEditor;
			
			// ノードIDを適用
			l_editorNodeEditor.ApplyNodeID(l_nodeEditorAllocator);
			
			l_notifyComponentEventExecutionConditionList.emplace_back(l_executionCondition);

			continue;
		}
		else
		{
			if (l_itr != l_notifyComponentEventExecutionConditionList.end())
			{
				auto& l_editorNodeEditor = l_itr->m_editorNodeEditor;

				// 管理している全てのNodeIDを削除する
				l_editorNodeEditor.Release(l_nodeEditorAllocator);

				l_notifyComponentEventExecutionConditionList.erase(l_itr);
			}
		}
	}

	ImGui::EndListBox();
	ImGui::EndGroup  ();
	ImGui::PopID     ();
}
void FWK::InputComponentInspector::DrawStartNode()
{
}
void FWK::InputComponentInspector::DrawExecutionConditionNodeList(InputComponent& a_inputComponent)
{

}

bool FWK::InputComponentInspector::PrepareExecutionConditionNode(Struct::ObserverInputExecutionCondition<Enum::ComponentEvent>& a_executionCondition)
{
	return false;
}
