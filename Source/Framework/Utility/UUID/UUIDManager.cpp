#include "UUIDManager.h"

boost::uuids::uuid FWK::Utility::UUIDManager::GenerateVALUUID()
{
    return m_randomGenerator();
}