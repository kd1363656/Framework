#include "InputComponentInspector.h"

void FWK::InputComponentInspector::INIT()
{
	m_nodeEditor.INIT();
}

void FWK::InputComponentInspector::EditInspector(InputComponent& a_inputComponent)
{
	m_nodeEditor.Begin(k_nodeEditorLabel);

	m_nodeEditor.End();
}