#include "GameObjectTransformComponentMatrixUpdateModeBaseInspector.h"

void FWK::GameObjectTransformComponentMatrixUpdateModeBaseInspector::EditInspector(GameObjectTransformComponentMatrixUpdateModeBase& a_gameObjectTransformComponentMatrixUpdateModeBase)
{
    bool l_isRotateAroundPosition = a_gameObjectTransformComponentMatrixUpdateModeBase.GetVALIsRotateAroundPosition();

    if (ImGui::Checkbox(k_isRotateAroundPositionJsonKey.data(), &l_isRotateAroundPosition))
    {
        a_gameObjectTransformComponentMatrixUpdateModeBase.SetIsRotateAroundPosition(l_isRotateAroundPosition);
    }
}