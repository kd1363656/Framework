#include "PhysicsStaticSphereBody.h"

bool FWK::Physics::PhysicsStaticSphereBody::CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, bool a_isPushBackEnabled)
{
	const auto& l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticSphereBody用Shapeが無効なため、Bodyの作成に失敗しました。", false);

	return CreateAndAddStaticBody(l_shape,
		                          a_worldPosition, 
		                          a_worldRotation, 
		                          a_isPushBackEnabled);
}

bool FWK::Physics::PhysicsStaticSphereBody::ApplyWorldTransform(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Quaternion& a_worldRotation)
{
	return ApplyStaticBodyWorldTransform(a_worldPosition, a_worldRotation);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticSphereBody::CreateShape() const
{
	FWK_ASSERT_RETURN_VALUE_IF(m_radius <= k_minRadius, "StaticSphereBodyのRadiusが0以下のため、Shapeの作成に失敗しました。", {});

	const JPH::SphereShapeSettings l_sphereShapeSettings{ m_radius };
	const auto&                    l_shapeResult = l_sphereShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "StaticSphereBody用SphereShapeの作成に失敗しました。", {});

	return l_shapeResult.Get();
}

bool FWK::Physics::PhysicsStaticSphereBody::ApplyShapeChange()
{
	const auto& l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticSphereBody用Shapeが無効なため、Shapeの変更に失敗しました。", false);

	return ApplyStaticBodyShape(l_shape);
}