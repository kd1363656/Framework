#include "ComponentBase.h"

void FWK::ComponentBase::Enable()
{
	if (!m_isDisable) { return; }

	m_isDisable = false;
}
void FWK::ComponentBase::Disable()
{
	if (m_isDisable) { return; }

	m_isDisable = true;
}