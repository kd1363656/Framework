#include "InputComponent.h"

void FWK::InputComponent::INIT()
{
	m_inspector.INIT();
}

void FWK::InputComponent::EditInspector()
{
	m_inspector.EditInspector(*this);
}