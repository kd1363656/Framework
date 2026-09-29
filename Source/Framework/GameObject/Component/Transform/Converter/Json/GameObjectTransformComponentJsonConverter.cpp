#include "GameObjectTransformComponentJsonConverter.h"

void FWK::Converter::GameObjectTransformComponentJsonConverter::Deserialize(const nlohmann::json& a_rootJson, GameObjectTransformComponent& a_transformComponent) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_scale    = Utility::DeserializeVector3   (a_rootJson, k_scaleJsonKey);
    const auto& l_rotation = Utility::DeserializeQuaternion(a_rootJson, k_rotationJsonKey);
    const auto& l_position = Utility::DeserializeVector3   (a_rootJson, k_positionJsonKey);

    a_transformComponent.ApplyTransformScale   (l_scale);
    a_transformComponent.ApplyTransformRotation(l_rotation);
    a_transformComponent.ApplyTransformPosition(l_position);

    auto& l_matrixUpdateMode = a_transformComponent.GetMutableREFMatrixUpdateMode();

    Utility::DeserializeInstanceType<TypeAlias::GameObjectTransformComponentMatrixUpdateModeUniqueFactory>(a_rootJson, k_matrixUpdateModeBaseJsonKey, l_matrixUpdateMode);

    if (!l_matrixUpdateMode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "TransformComponentのMatrixUpdateModeのデシリアライズに失敗。");
    }
}

nlohmann::json FWK::Converter::GameObjectTransformComponentJsonConverter::Serialize(const GameObjectTransformComponent& a_gameObjectTransformComponent) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_transform        = a_gameObjectTransformComponent.GetREFTransform       ();
    const auto& l_matrixUpdateMode = a_gameObjectTransformComponent.GetREFMatrixUpdateMode();

    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(l_transform.m_scale,       k_scaleJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(l_transform.m_rotation, k_rotationJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(l_transform.m_position,    k_positionJsonKey));
    
    // 行列更新モードのシリアライズ
    Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(l_matrixUpdateMode, k_matrixUpdateModeBaseJsonKey));

    return l_rootJson;
}