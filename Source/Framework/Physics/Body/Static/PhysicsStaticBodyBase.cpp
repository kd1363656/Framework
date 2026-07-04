#include "PhysicsStaticBodyBase.h"

FWK::Physics::PhysicsStaticBodyBase::PhysicsStaticBodyBase(const JPH::BodyID a_bodyID) : 
	PhysicsBodyBase(a_bodyID)
{}

FWK::Physics::PhysicsStaticBodyBase::PhysicsStaticBodyBase () = default;
FWK::Physics::PhysicsStaticBodyBase::~PhysicsStaticBodyBase() = default;

JPH::BodyID FWK::Physics::PhysicsStaticBodyBase::CreateAndAddStaticBody(const JPH::RefConst<JPH::Shape>& a_shape, const TypeAlias::Math::Vector3& a_worldPosition)
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_shape, "StaticBody用Shapeが無効なため、Bodyの作成に失敗しました。", {});

	      auto& l_physicsManager      = Physics::PhysicsManager::GetInstance       ();
	const auto& l_physicsLayerSetting = l_physicsManager.GetREFPhysicsLayerSetting ();
	      auto& l_physicsSystem       = l_physicsManager.GetMutableREFPhysicsSystem();
	
	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、Bodyの作成に失敗しました。", {});

	// StaticBodyは重力や速度では動かず
	// 床や壁などの固定Colliderとして使用する
	JPH::BodyCreationSettings l_bodyCreationSettings = { a_shape.GetPtr(),
								                         Utility::DirectXMathVector3ToJoltRVec3(a_worldPosition),
								                         JPH::Quat::sIdentity(),
								                         JPH::EMotionType::Static,
								                         l_physicsLayerSetting->FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	auto& l_bodyInterface = l_physicsSystem.GetBodyInterface();

	// StaticBodyは動かないため、作成直後にActivateする必要はない
	return l_bodyInterface.CreateAndAddBody(l_bodyCreationSettings, JPH::EActivation::DontActivate);
}