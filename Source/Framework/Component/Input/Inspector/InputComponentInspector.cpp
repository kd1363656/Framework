#include "InputComponentInspector.h"

void FWK::InputComponentInspector::INIT()
{
	m_nodeEditor.INIT();
}

void FWK::InputComponentInspector::EditInspector(InputComponent& a_inputComponent)
{
	auto* l_editorContext = m_nodeEditor.GetMutablePTREditorContext();

	// imgui-node-editorのContextがまだ生成されていない場合は
	// NodeEditorを描画できないため何もしない
	if (!l_editorContext)
	{
		ImGui::TextDisabled(Constant::k_missingNodeEditorContextLabel.data());

		return;
	}

	// imgu-node-editorでは、Begin()を呼ぶ前に
	// どのEditorContextを使用するか指定する必要がある
	ax::NodeEditor::SetCurrentEditor(l_editorContext);

	// DetailsEditorWindowのComponent領域内へ
	// NodeEditorのキャンバスを作成する
	ax::NodeEditor::Begin(k_nodeEditorName.data(), Constant::k_defaultNodeEditorCanvasSize);

	ax::NodeEditor::End();

	// 他のEditorへNodeEditorのContextが残らないように
	// 病が終了後は現在Contextを解除する
	ax::NodeEditor::SetCurrentEditor(nullptr);
}