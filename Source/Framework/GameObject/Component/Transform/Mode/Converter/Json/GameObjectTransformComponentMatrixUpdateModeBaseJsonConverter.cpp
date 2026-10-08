#include "GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter.h"

void FWK::Converter::GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponentMatrixUpdateModeBase& a_gameObjectTransformComponentMatrixUpdateModeBase) const
{
    if (a_rootJson.is_null()) { return; }

    const bool l_isRotateAroundPosition = a_rootJson.value(k_isRotateAroundPositionJsonKey, Constant::k_gameObjectTransformComponentMatrixUpdateModeBaseInitialVALIsRotateAroundPosition);

    a_gameObjectTransformComponentMatrixUpdateModeBase.SetIsRotateAroundPosition(l_isRotateAroundPosition);
}

nlohmann::json FWK::Converter::GameObjectTransformComponentMatrixUpdateModeBaseJsonConverter::Serialize(const GameObjectTransformComponentMatrixUpdateModeBase& a_gameObjectTransformComponentMatrixUpdateModeBase) const
{
          nlohmann::json l_rootJson               = {};
    const bool           l_isRotateAroundPosition = a_gameObjectTransformComponentMatrixUpdateModeBase.GetVALIsRotateAroundPosition();

    l_rootJson[k_isRotateAroundPositionJsonKey] = l_isRotateAroundPosition;

    return l_rootJson;
}