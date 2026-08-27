#include "NodeEditorNode.h"

void FWK::Editor::NodeEditorNode::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Editor::NodeEditorNode::ApplyNodeID(NodeEditorAllocator& a_nodeEditorAllocator)
{
	FWK_ASSERT_RETURN_VALUE_IF(FetchVALIsCreated(), "作成済みNodeEditorNodeへ再度NodeIDを割り当てようとしました。", false);

	const auto l_nodeID = a_nodeEditorAllocator.Allocate();

	if (l_nodeID == Constant::k_invalidNodeEditorID) { return false; }

	m_nodeID = l_nodeID;

	return true;
}

nlohmann::json FWK::Editor::NodeEditorNode::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

void FWK::Editor::NodeEditorNode::Release(NodeEditorAllocator& a_nodeEditorAllocator)
{
	// InputPinをすべて解放する
	ReleaseInputPinIDList(a_nodeEditorAllocator);

	// OutputPinをすべて解放する
	ReleaseOutputPinIDList(a_nodeEditorAllocator);

	if (FetchVALIsCreated())
	{
		a_nodeEditorAllocator.Release(m_nodeID);
	}

	Reset();
}

void FWK::Editor::NodeEditorNode::ReleaseInputPinIDList(NodeEditorAllocator& a_nodeEditorAllocator)
{
	for (const auto l_pinID : m_inputPInIDList)
	{
		a_nodeEditorAllocator.Release(l_pinID);
	}

	m_inputPInIDList.clear();
}
void FWK::Editor::NodeEditorNode::ReleaseOutputPinIDList(NodeEditorAllocator& a_nodeEditorAllocator)
{
	for (const auto l_pinID : m_outputPInIDList)
	{
		a_nodeEditorAllocator.Release(l_pinID);
	}

	m_outputPInIDList.clear();
}

bool FWK::Editor::NodeEditorNode::AddInputPinID(NodeEditorAllocator& a_nodeEditorAllocator)
{
	FWK_ASSERT_RETURN_VALUE_IF(!FetchVALIsCreated(), "NodeEditorNode作成前にInputPinを追加しようとしました。", false);

	const auto l_pinID = a_nodeEditorAllocator.Allocate();

	if (l_pinID == Constant::k_invalidNodeEditorID) { return false; }
	if (AddInputPinID(l_pinID))                     { return true; }

	// 追加に失敗していたらアロケーターへ戻す
	a_nodeEditorAllocator.Release(l_pinID);

	return false;
}
bool FWK::Editor::NodeEditorNode::AddInputPinID(const TypeAlias::NodeEditorID a_pinID)
{
	if (a_pinID == Constant::k_invalidNodeEditorID) { return false; }

	return AddPinIDToPinIDList(a_pinID, m_inputPInIDList);
}

bool FWK::Editor::NodeEditorNode::AddOutputPinID(NodeEditorAllocator& a_nodeEditorAllocator)
{
	FWK_ASSERT_RETURN_VALUE_IF(!FetchVALIsCreated(), "NodeEditorNode作成前にOutputPinを追加しようとしました。", false);

	const auto l_pinID = a_nodeEditorAllocator.Allocate();

	if (l_pinID == Constant::k_invalidNodeEditorID) { return false; }
	if (AddOutputPinID(l_pinID))                    { return true; }

	// 追加に失敗していたらアロケーターへ戻す
	a_nodeEditorAllocator.Release(l_pinID);
	
	return false;
}
bool FWK::Editor::NodeEditorNode::AddOutputPinID(const TypeAlias::NodeEditorID a_pinID)
{
	if (a_pinID == Constant::k_invalidNodeEditorID) { return false; }

	return AddPinIDToPinIDList(a_pinID, m_outputPInIDList);
}

bool FWK::Editor::NodeEditorNode::FetchVALIsCreated() const
{
	return m_nodeID != Constant::k_invalidNodeEditorID;
}

bool FWK::Editor::NodeEditorNode::AddPinIDToPinIDList(const TypeAlias::NodeEditorID a_pinID, std::vector<TypeAlias::NodeEditorID>& a_pinIDList) const
{
	// 重複したIDを持つとImNodesが正常に動作しないので同じIDがリストに含まれているかどうかを
	// 確認して追加するかどうかを決めるもし存在するならfalseを返す
	if (const auto& l_itr = std::find_if(a_pinIDList.begin(), a_pinIDList.end(), [&a_pinID](const auto& a_listOutputPin)
		{
			return a_pinID == a_listOutputPin;
		});
		l_itr != a_pinIDList.end())
	{
		return false;
	}

	a_pinIDList.emplace_back(a_pinID);

	return true;
}

void FWK::Editor::NodeEditorNode::Reset()
{
	m_inputPInIDList.clear ();
	m_outputPInIDList.clear();

	m_nodePosition = {};

	m_nodeID = Constant::k_invalidNodeEditorID;
}