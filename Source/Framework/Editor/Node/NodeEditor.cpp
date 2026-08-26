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

bool FWK::Editor::NodeEditor::Begin(const std::string_view& a_editorLabel, const ImVec2 a_canvasSize)
{
	// imgui-node-editorのContextがまだ生成されていない場合は
	// NodeEditorを描画できないため何もしない
	if (!m_editorContext)
	{
		ImGui::TextDisabled(k_missingNodeEditorContextLabel.data());

		return false;
	}

	// imgu-node-editorでは、Begin()を呼ぶ前に
	// どのEditorContextを使用するか指定する必要がある
	ax::NodeEditor::SetCurrentEditor(m_editorContext);

	// DetailsEditorWindowのComponent領域内へ
	// NodeEditorのキャンバスを作成する
	ax::NodeEditor::Begin(a_editorLabel.data(), a_canvasSize);
}

void FWK::Editor::NodeEditor::End()
{
	ax::NodeEditor::End();

	// 他のEditorへNodeEditorのContextが残らないように
	// 病が終了後は現在Contextを解除する
	ax::NodeEditor::SetCurrentEditor(nullptr);
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