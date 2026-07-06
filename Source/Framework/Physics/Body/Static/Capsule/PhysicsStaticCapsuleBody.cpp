#include "PhysicsStaticCapsuleBody.h"

bool FWK::Physics::PhysicsStaticCapsuleBody::CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, bool a_isPushBackEnabled)
{
	const auto& l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticCapsuleBody用Shapeが無効なため、Bodyの作成に失敗しました。", false);

	return CreateAndAddStaticBody(l_shape, 
		                          a_worldPosition,
		                          a_worldRotation,
		                          a_isPushBackEnabled);
}

bool FWK::Physics::PhysicsStaticCapsuleBody::ApplyWorldTransform(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Quaternion& a_worldRotation)
{
	return ApplyStaticBodyWorldTransform(a_worldPosition, a_worldRotation);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticCapsuleBody::CreateShape() const
{
	FWK_ASSERT_RETURN_VALUE_IF(m_halfHeightOfCylinder <= k_minHalfHeightOfCylinder, "StaticCapsuleBodyのHalfHeightOfCylinderが0以下のため、Shapeの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(m_radius <= k_minRadius,                             "StaticCapsuleBodyのRadiusが0以下のため、Shapeの作成に失敗しました。",               {});

	const JPH::CapsuleShapeSettings& l_capsuleShapeSettings = { m_halfHeightOfCylinder, m_radius };

	const auto& l_shapeResult = l_capsuleShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "StaticCapsuleBody用CapsuleShapeの作成に失敗しました。", {});

	return l_shapeResult.Get();
}

bool FWK::Physics::PhysicsStaticCapsuleBody::ApplyShapeChange()
{
	const auto l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticCapsuleBody用Shapeが無効なため、Shapeの変更に失敗しました。", false);

	return ApplyStaticBodyShape(l_shape);
}