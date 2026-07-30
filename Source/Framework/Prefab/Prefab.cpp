#include "Prefab.h"

nlohmann::json FWK::Prefab::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}