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

    Utility::DeserializeInstanceType<TypeAlias::GameObjectTransformComponentMatrixUpdateModeUniqueFactory>(a_rootJson, k_matrixUpdateModeJsonKey, l_matrixUpdateMode);

    // インスタンス化出来なければreturn(デシリアライズの意味がないから)
    if (!l_matrixUpdateMode)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "TransformComponentのMatrixUpdateModeのデシリアライズに失敗。");

        return;
    }

    const auto& l_matrixUpdateModeDataJson = a_rootJson.value(k_matrixUpdateModeDataJsonKey, nlohmann::json{});

    if (l_matrixUpdateModeDataJson.is_null()) { return; }

    l_matrixUpdateMode->Deserialize(l_matrixUpdateModeDataJson);
}

nlohmann::json FWK::Converter::GameObjectTransformComponentJsonConverter::Serialize(const GameObjectTransformComponent& a_gameObjectTransformComponent) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_transform        = a_gameObjectTransformComponent.GetREFTransform       ();
    const auto& l_matrixUpdateMode = a_gameObjectTransformComponent.GetREFMatrixUpdateMode();

    Utility::UpdateJson(Utility::SerializeVector3(k_scaleJsonKey, l_transform.m_scale), l_rootJson);
    Utility::UpdateJson(Utility::SerializeQuaternion(k_rotationJsonKey, l_transform.m_rotation), l_rootJson);
    Utility::UpdateJson(Utility::SerializeVector3(k_positionJsonKey, l_transform.m_position), l_rootJson);

    if (!l_matrixUpdateMode) { return l_rootJson; }

    // 行列更新モードのシリアライズ
    Utility::UpdateJson(Utility::SerializeInstanceType(l_matrixUpdateMode, k_matrixUpdateModeJsonKey), l_rootJson);

    l_rootJson[k_matrixUpdateModeDataJsonKey] = l_matrixUpdateMode->Serialize();

    return l_rootJson;
}