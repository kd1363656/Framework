#include "NodeEditor.h"

FWK::Editor::NodeEditor::NodeEditor() : 
	m_editorContext(nullptr)
{}
FWK::Editor::NodeEditor::~NodeEditor()
{
	Release();
}

void FWK::Editor::NodeEditor::INIT()
{
	// 同じクラスからEditorContextを二重生成すると
	// 最初に生成したEditorContextを失ってしまうため禁止
	FWK_ASSERT_RETURN_IF(m_editorContext, "IMGUINodeEditorContextはすでに作成されています。");

	ax::NodeEditor::Config l_config = {};

	l_config.SettingsFile = nullptr;

	m_editorContext = ax::NodeEditor::CreateEditor(&l_config);

	FWK_ASSERT_RETURN_IF(!m_editorContext, "imgui-node-editorのEditorContext生成に失敗しました。");
}

void FWK::Editor::NodeEditor::Release()
{
	if (!m_editorContext) { return; }
	
	// 現在使用中になっているEditorContextをそのまま破棄すると、
	// imgui-node-editor側に破棄済みContextが現在値として
	// 残る可能性を避けたいので、先にnullptrへ戻す
	if (ax::NodeEditor::GetCurrentEditor() == m_editorContext)
	{
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	ax::NodeEditor::DestroyEditor(m_editorContext);

	m_editorContext = nullptr;
}