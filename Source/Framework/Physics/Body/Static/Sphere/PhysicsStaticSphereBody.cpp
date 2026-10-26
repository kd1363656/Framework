#include "PhysicsStaticSphereBody.h"

bool FWK::Physics::PhysicsStaticSphereBody::CreateBody(const float                        a_radius,
                                                       const TypeAlias::Math::Quaternion& a_worldRotation,
                                                       const TypeAlias::Math::Vector3&    a_worldPosition,
                                                       const bool                         a_isPushBackEnabled)
{
    const auto& l_shape = CreateShape(a_radius);

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticSphereBody用Shapeが無効なため、Bodyの作成に失敗しました。", false);

    return CreateAndAddStaticBody(l_shape,
                                  a_worldPosition,
                                  a_worldRotation,
                                  a_isPushBackEnabled);
}

bool FWK::Physics::PhysicsStaticSphereBody::ApplyShapeChange(const float a_radius)
{
    const auto& l_shape = CreateShape(a_radius);

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticSphereBody用Shapeが無効なため、Shapeの変更に失敗しました。", false);

    return ApplyStaticBodyShape(l_shape);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticSphereBody::CreateShape(const float a_radius) const
{
    FWK_ASSERT_RETURN_VALUE_IF(a_radius <= std::numeric_limits<float>::epsilon(), "StaticSphereBodyのRadiusが0以下のため、Shapeの作成に失敗しました。", {});

    const JPH::SphereShapeSettings l_sphereShapeSettings{ a_radius };
    const auto&                    l_shapeResult = l_sphereShapeSettings.Create();

    FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "StaticSphereBody用SphereShapeの作成に失敗しました。", {});

    return l_shapeResult.Get();
}