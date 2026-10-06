#include "GameObjectTransformComponentMatrixUpdateModeBase.h"

void FWK::GameObjectTransformComponentMatrixUpdateModeBase::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::GameObjectTransformComponentMatrixUpdateModeBase::EditInspector()
{
    m_inspector.EditInspector(*this);
}

nlohmann::json FWK::GameObjectTransformComponentMatrixUpdateModeBase::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}