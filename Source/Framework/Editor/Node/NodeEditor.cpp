#include "NodeEditor.h"

FWK::Editor::NodeEditor::NodeEditor() :
	m_editorContext(nullptr),

	m_isDrawing(false)
{}
FWK::Editor::NodeEditor::~NodeEditor()
{
	Release();
}

void FWK::Editor::NodeEditor::INIT()
{
	// 同じNodeEditorからContextを二重生成しない
	if (m_editorContext) { return; }

	// EditorManager側でImNodes全体のContextが
	// 先に生成されている必要がある
	FWK_ASSERT_RETURN_IF(!ImNodes::GetCurrentContext(), "ImNodesContextが存在しないため、NodeEditorを初期化できませんでした。" );

	// このNodeGraphだけが使用するEditorContextを作成する
	// こうすることで他のNodeEditorとNodeIDが寿福しても問題ない
	m_editorContext = ImNodes::EditorContextCreate();

	FWK_ASSERT_RETURN_IF(!m_editorContext, "ImNodesEditorContextの作成に失敗したため、NodeEditorを初期化できませんでした。" );
}

void FWK::Editor::NodeEditor::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::Editor::NodeEditor::BeginDraw()
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_editorContext, "ImNodesEditorContextが存在しないため、NodeEditorの描画を開始できませんでした。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_isDrawing,      "NodeEditorの描画処理が既に開始されています。",                                   false);

	// このNodeEditor専用Contextへ切り替える
	ImNodes::EditorContextSet(m_editorContext);
	ImNodes::BeginNodeEditor ();

	m_isDrawing = true;

	return true;
}

void FWK::Editor::NodeEditor::DrawLinkList() const
{
	for (const auto& l_linkData : m_linkDataList)
	{
		ImNodes::Link(l_linkData.m_linkID, l_linkData.m_outputPinID, l_linkData.m_inputPinID);
	}
}

void FWK::Editor::NodeEditor::EndDraw()
{
	if (!m_isDrawing) { return; }

	ImNodes::EndNodeEditor();

	m_isDrawing = false;
}

nlohmann::json FWK::Editor::NodeEditor::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Editor::NodeEditor::ApplyNodePosition(const NodeEditorNode& a_nodeEditorNode)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_editorContext, "ImNodesEditorContextが存在しないため、Node座標を反映できませんでした。", false);

	if (!a_nodeEditorNode.FetchVALIsCreated()) { return false; }

	ImNodes::EditorContextSet   (m_editorContext);
	ImNodes::SetNodeGridSpacePos(a_nodeEditorNode.GetVALNodeID(), a_nodeEditorNode.GetREFNodePosition());

	return true;
}

bool FWK::Editor::NodeEditor::AddLink(const TypeAlias::NodeEditorID a_inputPinID, const TypeAlias::NodeEditorID a_outputPinID)
{
	if (a_inputPinID == Constant::k_invalidNodeEditorID ||
		a_outputPinID == Constant::k_invalidNodeEditorID)
	{
		return false;
	}

	// 自分自身のPinへLinkすることは許可しない
	if (a_inputPinID == a_outputPinID) { return false; }

	if (FetchVALHasLink(a_inputPinID, a_outputPinID)) { return false; }

	// Node・Pin・Link全てのIDを同じAllocatorから払い出す
	// こうすることでNodeEditor内部に同じIDが存在しないようにする
	const auto l_linkID = m_nodeEditorAllocator.Allocate();

	if (l_linkID == Constant::k_invalidNodeEditorID) { return false; }

	Struct::NodeEditorLinkData l_linkData = {};

	l_linkData.m_linkID      = l_linkID;
	l_linkData.m_inputPinID  = a_inputPinID;
	l_linkData.m_outputPinID = a_outputPinID;

	if (AddLink(l_linkData)) { return true; }

	// Link追加に失敗した場合、
	// 確保したLinkIDを使用中のまま残さない
	m_nodeEditorAllocator.Release(l_linkID);

	return false;
}
bool FWK::Editor::NodeEditor::AddLink(const Struct::NodeEditorLinkData& a_linkData)
{
	if (a_linkData.m_linkID == Constant::k_invalidNodeEditorID     ||
		a_linkData.m_inputPinID == Constant::k_invalidNodeEditorID ||
		a_linkData.m_outputPinID == Constant::k_invalidNodeEditorID)
	{
		return false;
	}

	// 同じLinkIDが複数存在すると
	// ImNodesがLinkを一意に識別できなくなる
	if (std::ranges::any_of(m_linkDataList, [&a_linkData](const auto& a_listLinkData) 
		                   {
								return a_listLinkData.m_linkID == a_linkData.m_linkID;
		                   }))
	{
		return false;
	}

	// 全く同じOutputPin->InputPinのLinkも重複して保持しない
	if (FetchVALHasLink(a_linkData.m_inputPinID, a_linkData.m_outputPinID)) { return false; }

	m_linkDataList.emplace_back(a_linkData);

	return true;
}

void FWK::Editor::NodeEditor::RemoveLink(const TypeAlias::NodeEditorID a_linkID)
{
	const auto& l_itr = std::find_if(m_linkDataList.begin(), 
		                             m_linkDataList.end(),
			                         [a_linkID](const auto& a_linkData)
			                         {
										return a_linkData.m_linkID == a_linkID;
			                         });

	if (l_itr == m_linkDataList.end()) { return; }

	// LinkIDもNodeEditorAllocatorから取得しているので、
	// Link削除時には必ずAllocatorへ返却する。
	m_nodeEditorAllocator.Release(l_itr->m_linkID);

	m_linkDataList.erase(l_itr);
}

void FWK::Editor::NodeEditor::ClearLinkData()
{
	m_linkDataList.clear();
}

bool FWK::Editor::NodeEditor::FetchVALIsInitialized() const
{
	if (m_editorContext) { return true; }

	return false;
}

bool FWK::Editor::NodeEditor::FetchVALIsInputPinLinked(const TypeAlias::NodeEditorID a_inputPinID) const
{
	return std::ranges::any_of(m_linkDataList, [&a_inputPinID](const auto& a_linkData)
		                       {
									return a_linkData.m_inputPinID == a_inputPinID;
		                       });
}
bool FWK::Editor::NodeEditor::FetchVALHasLink(const TypeAlias::NodeEditorID a_inputPinID, const TypeAlias::NodeEditorID a_outputPinID) const
{
	return std::ranges::any_of(m_linkDataList, [a_inputPinID, a_outputPinID](const auto& a_linkData)
		                       {
									const bool l_isSameOutputPin = a_linkData.m_outputPinID == a_outputPinID;
									const bool l_isSameInputPin  = a_linkData.m_inputPinID  == a_inputPinID;

									return l_isSameOutputPin && l_isSameInputPin;
		                       });
}

void FWK::Editor::NodeEditor::Release()
{
	if (!m_editorContext) { return; }

	// NodeEditorの破棄が描画途中で発生する設計にはしない。
	FWK_ASSERT_RETURN_IF(m_isDrawing, "NodeEditorの描画途中にImNodesEditorContextを解放しようとしました。" );

	// EditorContextCreate()で作成したContextは
	// ImNodes::DestroyContext()では自動解放されないため
	// 所有しているNodeEditor自身が解放する
	ImNodes::EditorContextFree(m_editorContext);

	m_editorContext = nullptr;
}