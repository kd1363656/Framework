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

bool FWK::Editor::NodeEditor::PrepareDraw()
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_editorContext, "ImNodesEditorContextが存在しないため、NodeEditorの描画を開始できませんでした。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_isDrawing,      "NodeEditorの描画処理が既に開始されています。",                                   false);

	// このNodeEditor専用Contextへ切り替える
	ImNodes::EditorContextSet(m_editorContext);
	ImNodes::BeginNodeEditor ();

	m_isDrawing = true;

	return true;
}

void FWK::Editor::NodeEditor::FinalizeDraw()
{
	if (!m_isDrawing) { return; }

	ImNodes::EndNodeEditor();

	m_isDrawing = false;
}

bool FWK::Editor::NodeEditor::FetchVALIsInitialized() const
{
	if (m_editorContext) { return true; }

	return false;
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