#include "PhysicsStaticBodyBase.h"

bool FWK::Physics::PhysicsStaticBodyBase::CreateAndAddStaticBody(const JPH::RefConst<JPH::Shape>&   a_shape, 
	                                                             const TypeAlias::Math::Vector3&    a_worldPosition, 
	                                                             const TypeAlias::Math::Quaternion& a_worldRotation, 
	                                                             const bool                         a_isPushBackEnabled)
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_shape,                    "StaticBody用Shapeが無効なため、Bodyの作成に失敗しました。",      false);
	FWK_ASSERT_RETURN_VALUE_IF(!GetREFBodyID().IsInvalid(), "StaticBodyが既に作成されているため、Bodyの作成に失敗しました。", false);

	JPH::Quat l_physicsWorldRotation = { a_worldRotation.x,
										 a_worldRotation.y, 
										 a_worldRotation.z,
										 a_worldRotation.w };

	FWK_ASSERT_RETURN_VALUE_IF(l_physicsWorldRotation.IsNaN() ||
		                       l_physicsWorldRotation.LengthSq() <= std::numeric_limits<float>::epsilon(),
		                       "WorldRotationが無効となっており、Bodyの作成に失敗しました。", 
		                       false);

	      auto& l_physicsManager      = Physics::PhysicsManager::GetInstance       ();
	const auto& l_physicsLayerSetting = l_physicsManager.GetREFPhysicsLayerSetting ();
	      auto& l_physicsSystem       = l_physicsManager.GetMutableREFPhysicsSystem();
	
	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、Bodyの作成に失敗しました。", false);

	// Joltへ渡すQuaternionは単位Quaternionにそろえる
	l_physicsWorldRotation = l_physicsWorldRotation.Normalized();

	JPH::BodyCreationSettings l_bodyCreationSettings = { a_shape.GetPtr(),
	                                                     Utility::DirectXMathVector3ToJoltRVec3(a_worldPosition),
	                                                     l_physicsWorldRotation,
														 JPH::EMotionType::Static,
	                                                     l_physicsLayerSetting->FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	// trueなら通常Collider
	// falseならSensorとして作成し、押し戻しを行わない
	l_bodyCreationSettings.mIsSensor = !a_isPushBackEnabled;

	auto& l_bodyInterface = l_physicsSystem.GetBodyInterface();

	const auto& l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreationSettings, JPH::EActivation::DontActivate);

	FWK_ASSERT_RETURN_VALUE_IF(l_bodyID.IsInvalid(), "StaticBodyの作成に失敗しました。", false);

	SetBodyID(l_bodyID);

	return true;
}

bool FWK::Physics::PhysicsStaticBodyBase::ApplyStaticBodyWorldTransform(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition)
{
	FWK_ASSERT_RETURN_VALUE_IF(GetREFBodyID().IsInvalid(), "StaticBodyが作成されていないため、WorldTransformの反映に失敗しました。", false);

	JPH::Quat l_physicsWorldRotation = { a_worldRotation.x,
										 a_worldRotation.y,
										 a_worldRotation.z,
										 a_worldRotation.w };

	FWK_ASSERT_RETURN_VALUE_IF(l_physicsWorldRotation.IsNaN() ||
		                       l_physicsWorldRotation.LengthSq() <= std::numeric_limits<float>::epsilon(),
		                       "StaticBodyのWorldRotationが無効なため、WorldTransformの反映に失敗しました。",
		                       false);

	l_physicsWorldRotation = l_physicsWorldRotation.Normalized();

	auto& l_physicsManager = Physics::PhysicsManager::GetInstance       ();
	auto& l_physicsSystem  = l_physicsManager.GetMutableREFPhysicsSystem();
	auto& l_bodyInterface  = l_physicsSystem.GetBodyInterface           ();

	// 外部で計算された最終ワールド座標とワールド回転を、
	// StaticBodyへそのまま反映する
	l_bodyInterface.SetPositionAndRotationWhenChanged(GetREFBodyID(), 
		                                              Utility::DirectXMathVector3ToJoltRVec3(a_worldPosition), 
		                                              l_physicsWorldRotation,
													  JPH::EActivation::DontActivate);
	
	return true;
}

bool FWK::Physics::PhysicsStaticBodyBase::ApplyStaticBodyShape(const JPH::RefConst<JPH::Shape>& a_shape)
{
	// StaticBodyには質量と歓声を使った移動処理がないため、
	// Shape交換時の質量再計算は行わない
	return ApplyBodyShape(a_shape, JPH::EActivation::DontActivate, false);
}