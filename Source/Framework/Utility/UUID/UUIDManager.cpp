#include "UUIDManager.h"

boost::uuids::uuid FWK::UUIDManager::GenerateVALUUID()
{
	return m_randomGenerator();
}