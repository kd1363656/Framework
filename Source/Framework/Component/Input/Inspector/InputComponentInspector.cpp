#include "InputComponentInspector.h"

void FWK::InputComponentInspector::INIT()
{
	m_nodeEditor.INIT();
}

void FWK::InputComponentInspector::EditInspector(InputComponent& a_inputComponent)
{
	// DetailsEditorWindow内部でNodeEditorが
	// 必要以上に縦へ広がらないよう固定高さのChildを作る
	ImGui::BeginChild(k_nodeEditorLabel.data(), Constant::k_defaultNodeEditorCanvasSize);

	if (m_nodeEditor.PrepareDraw())
	{
		m_nodeEditor.FinalizeDraw();
	}

	ImGui::EndChild();
}