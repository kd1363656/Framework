#include "UUIDManager.h"

FWK::TypeAlias::UUID FWK::UUIDManager::GenerateVALUUID()
{
	return m_randomGenerator();
}