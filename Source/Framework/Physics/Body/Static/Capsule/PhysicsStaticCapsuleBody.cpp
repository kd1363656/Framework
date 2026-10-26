#include "PhysicsStaticCapsuleBody.h"

bool FWK::Physics::PhysicsStaticCapsuleBody::CreateBody(const float                        a_halfHeightOfCylinder,
                                                        const float                        a_radius,
                                                        const TypeAlias::Math::Quaternion& a_worldRotation,
                                                        const TypeAlias::Math::Vector3&    a_worldPosition,
                                                        const bool                         a_isPushBackEnabled)
{
    const auto& l_shape = CreateShape(a_halfHeightOfCylinder, a_radius);

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticCapsuleBody用Shapeが無効なため、Bodyの作成に失敗しました。", false);

    return CreateAndAddStaticBody(l_shape,
                                  a_worldPosition,
                                  a_worldRotation,
                                  a_isPushBackEnabled);
}

bool FWK::Physics::PhysicsStaticCapsuleBody::ApplyShapeChange(const float a_halfHeightOfCylinder, const float a_radius)
{
    const auto l_shape = CreateShape(a_halfHeightOfCylinder, a_radius);

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticCapsuleBody用Shapeが無効なため、Shapeの変更に失敗しました。", false);

    return ApplyStaticBodyShape(l_shape);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticCapsuleBody::CreateShape(const float a_halfHeightOfCylinder, const float a_radius) const
{
    FWK_ASSERT_RETURN_VALUE_IF(a_halfHeightOfCylinder <= std::numeric_limits<float>::epsilon(), "StaticCapsuleBodyのHalfHeightOfCylinderが0以下のため、Shapeの作成に失敗しました。", {});
    FWK_ASSERT_RETURN_VALUE_IF(a_radius               <= std::numeric_limits<float>::epsilon(), "StaticCapsuleBodyのRadiusが0以下のため、Shapeの作成に失敗しました。",               {});

    const JPH::CapsuleShapeSettings& l_capsuleShapeSettings = { a_halfHeightOfCylinder, a_radius };

    const auto& l_shapeResult = l_capsuleShapeSettings.Create();

    FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "StaticCapsuleBody用CapsuleShapeの作成に失敗しました。", {});

    return l_shapeResult.Get();
}