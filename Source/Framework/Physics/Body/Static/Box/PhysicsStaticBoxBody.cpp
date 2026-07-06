#include "PhysicsStaticBoxBody.h"

bool FWK::Physics::PhysicsStaticBoxBody::CreateBody(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled)
{
	const auto& l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticBoxBody用Shapeが無効なため、Bodyの作成に失敗しました。", false);

	return CreateAndAddStaticBody(l_shape, 
		                          a_worldPosition, 
		                          a_worldRotation, 
		                          a_isPushBackEnabled);
}

bool FWK::Physics::PhysicsStaticBoxBody::ApplyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition)
{
	return ApplyStaticBodyWorldTransform(a_worldRotation, a_worldPosition);
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsStaticBoxBody::CreateShape() const
{
	// いずれかの軸の半径が0以下ならBoxとして機能しないのでassert;
	FWK_ASSERT_RETURN_VALUE_IF(m_halfExtent.x <= std::numeric_limits<float>::epsilon() ||
		                       m_halfExtent.y <= std::numeric_limits<float>::epsilon() ||
		                       m_halfExtent.z <= std::numeric_limits<float>::epsilon(),
			                   "StaticBoxBodyのHalfExtentが0以下のため、作成に失敗しました。",
                               {});

	const auto&                 l_halfExtent = Utility::DirectXMathVector3ToJoltVec3(m_halfExtent);
	const JPH::BoxShapeSettings l_boxShapeSettingss{ l_halfExtent };

	const auto& l_shapeResult = l_boxShapeSettingss.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "StaticBoxBody用BoxShapeの作成に失敗しました。", {});

	return l_shapeResult.Get();
}

bool FWK::Physics::PhysicsStaticBoxBody::ApplyShapeChange()
{
	const auto l_shape = CreateShape();

	FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "StaticBoxBody用Shapeが無効なため、Shapeの変更に失敗しました。", false);

	return ApplyStaticBodyShape(l_shape);
}