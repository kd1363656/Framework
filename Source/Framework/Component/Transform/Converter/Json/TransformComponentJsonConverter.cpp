#include "TransformComponentJsonConverter.h"

void FWK::Converter::TransformComponentJsonConverter::Deserialize(const nlohmann::json& a_rootJson, TransformComponent& a_transformComponent) const
{
    if (a_rootJson.is_null()) { return; }

    const auto& l_scale    = Utility::DeserializeVector3   (a_rootJson, k_scaleJsonKey);
    const auto& l_rotation = Utility::DeserializeQuaternion(a_rootJson, k_rotationJsonKey);
    const auto& l_position = Utility::DeserializeVector3   (a_rootJson, k_positionJsonKey);

    a_transformComponent.ApplyTransformScale   (l_scale);
    a_transformComponent.ApplyTransformRotation(l_rotation);
    a_transformComponent.ApplyTransformPosition(l_position);
    
    auto& l_matrixStrategy = a_transformComponent.GetMutableREFMatrixStrategy();

    Utility::DeserializeInstanceType<TypeAlias::MatrixStrategyUniqueFactory>(a_rootJson, k_matrixStrategyTypeNameJsonKey, l_matrixStrategy);

    if (!l_matrixStrategy)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "トランスフォームコンポーネントでMatrixStrategyのデシリアライズ、処理に失敗しました。");
    }
}

nlohmann::json FWK::Converter::TransformComponentJsonConverter::Serialize(const TransformComponent& a_transformComponent) const
{
    nlohmann::json l_rootJson = {};

    const auto& l_transform      = a_transformComponent.GetREFTransform     ();
    const auto& l_matrixStrategy = a_transformComponent.GetREFMatrixStrategy();

    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(l_transform.m_scale,       k_scaleJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeQuaternion(l_transform.m_rotation, k_rotationJsonKey));
    Utility::UpdateJson(l_rootJson, Utility::SerializeVector3(l_transform.m_position,    k_positionJsonKey));

    Utility::UpdateJson(l_rootJson, Utility::SerializeInstanceType(l_matrixStrategy, k_matrixStrategyTypeNameJsonKey));

    return l_rootJson;
}