#include "PhysicsBodyCreator.h"

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
																					 const TypeAlias::Math::Vector3& a_worldPosition,
																					 const TypeAlias::Math::Vector3& a_halfExtent, 
																						   JPH::PhysicsSystem&       a_physicsSystem) const
{
	// X,Y,Zどれ一つでもHlfExtentが0以下の場合アサート
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_halfExtent.x <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.y <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.z <= k_minValidBoxHalfExtentLength,
									  "BoxBodyのHalfExtentが0以下のため、作成に失敗しました。",
									  {});

	const JPH::Vec3 l_halfExtent = ConvertToJoltVector3(a_halfExtent);

	// BoxのShape設定を作成
	JPH::BoxShapeSettings l_boxShapeSettings(l_halfExtent);

	// ShapeSettingsから実際のShapeを作成する
	const auto l_shapeResult = l_boxShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "BoxShapeの作成に失敗しました。", {});

	const auto l_shape = l_shapeResult.Get();

	// StaticObjectは床、壁、地形などの動かないBodyに使う
	// EMotionType::Static:
	// Jolt側で動かない物体として扱われる、重力や速度では移動しない
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
												    ConvertToJoltVector3(a_worldPosition),
												    JPH::Quat::sIdentity(),
												    JPH::EMotionType::Static,
												    a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// Bodyを作成し、同時にPhysicsSystemへ追加する
	// StaticBodyは動かないため、作成直後にActiveにはしない
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::DontActivate);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "StaticBoxBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateDynamicSphereBody(const PhysicsLayerSetting&      a_physicsLayerSetting, 
																						 const TypeAlias::Math::Vector3& a_worldPosition, 
																						 const float					 a_radius, 
																							   JPH::PhysicsSystem&       a_physicsSystem) const
{
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_radius <= k_minValidSphereRadius, "SphereBodyのRadiusが0以下のため、作成に失敗しました。", {});

	// SphereのShape設定を作る
	JPH::SphereShapeSettings l_sphereShapeSettings(a_radius);

	// ShapeSettingsから実際のShapeを作成する
	const auto l_shapeResult = l_sphereShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "SphereShapeの作成に失敗しました。", {});

	// 作成済みShapeを取得する
	const auto l_shape = l_shapeResult.Get();

	// DynamicBody用の作成設定を作る
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													ConvertToJoltVector3(a_worldPosition),
													JPH::Quat::sIdentity(),
													JPH::EMotionType::Dynamic,
													a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::DynamicObject) };
	// PhysicsSystemからBodyInterfaceを取得する
	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// Bodyを作成し、同時にPhysicsSystemへ追加する
	// DynamicBodyは重力で動かしたいので、作成直後にActiveにする
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::Activate);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "DynamicSphereBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

	return l_bodyHandle;
}

JPH::Vec3 FWK::Physics::PhysicsBodyCreator::ConvertToJoltVector3(const TypeAlias::Math::Vector3& a_vector) const
{
	// NowProject側のVector3をJolt側のVec3へ変換する
	return JPH::Vec3{ a_vector.x, a_vector.y, a_vector.z };
}
