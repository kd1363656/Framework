#include "PhysicsStaticCapsuleBody.h"

bool FWK::Physics::PhysicsStaticCapsuleBody::CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled)
{
	const auto& l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticCapsuleBody用Shapeが無効なため、Bodyの作成に失敗しました。", false);

	return CreateAndAddStaticBody(l_shape, 
		                          a_worldPosition,
		                          a_worldRotation,
		                          a_isPushBackEnabled);
}

void FWK::Physics::PhysicsStaticCapsuleBody::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

nlohmann::json FWK::Physics::PhysicsStaticCapsuleBody::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::Physics::PhysicsStaticCapsuleBody::ApplyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition)
{
	return ApplyStaticBodyWorldTransform(a_worldRotation, a_worldPosition);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticCapsuleBody::CreateShape() const
{
	FWK_ASSERT_RETURN_VALUE_IF(m_halfHeightOfCylinder <= std::numeric_limits<float>::epsilon(), "StaticCapsuleBodyのHalfHeightOfCylinderが0以下のため、Shapeの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(m_radius <= std::numeric_limits<float>::epsilon(),               "StaticCapsuleBodyのRadiusが0以下のため、Shapeの作成に失敗しました。",               {});

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