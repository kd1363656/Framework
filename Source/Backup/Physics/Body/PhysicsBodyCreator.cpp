#include "PhysicsBodyCreator.h"


FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticSphereBody(const PhysicsLayerSetting&	    a_physicsLayerSetting, 
																				        const TypeAlias::Math::Vector3& a_worldPosition,
																				        const float					    a_radius, 
																				        	  JPH::PhysicsSystem&       a_physicsSystem) const
{
	// 半径が0以下のSphereは、当たり判定として成立しない
	FWK_ASSERT_RETURN_VALUE_IF(a_radius <= k_minValidSphereRadius, "SphereBodyのRadiusが0以下のため、作成に失敗しました。", {});

	// 半径がどれぐらいの球を作るのかを伝える設定
	const JPH::SphereShapeSettings l_sphereShapeSettings{ a_radius };

	const auto& l_shapeResult = l_sphereShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "SphereBody用のSphereShapeの作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	// Static Bodyとして作成する。
	// Static Bodyは重力や速度によって移動せず、
	// 衝突対象となる固定Colliderとして使用される。
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													Utility::DirectXMathVector3ToJoltVec3(a_worldPosition),
													JPH::Quat::sIdentity(),
													JPH::EMotionType::Static,
													a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// ボディの作成
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::DontActivate);

	FWK_ASSERT_RETURN_VALUE_IF(l_bodyID.IsInvalid(), "SphereBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = true;

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
																					 const TypeAlias::Math::Vector3& a_worldPosition,
																					 const TypeAlias::Math::Vector3& a_halfExtent, 
																						   JPH::PhysicsSystem&       a_physicsSystem) const
{
	// BoxのHalfExtentは中央から各軸方向への半分の長さ
	// 例えば幅10,高さ2,奥行き4ならHalfExtentはx = 5, y = 1, z = 2
	// どれか一つでも0以下だと、厚みのない無効なBoxになる
	FWK_ASSERT_RETURN_VALUE_IF(a_halfExtent.x <= k_minValidBoxHalfExtentLength ||
							   a_halfExtent.y <= k_minValidBoxHalfExtentLength ||
							   a_halfExtent.z <= k_minValidBoxHalfExtentLength,
							   "BoxBodyのHalfExtentが0以下のため、作成に失敗しました。",
							   {});

	const JPH::Vec3& l_halfExtent = { a_halfExtent.x, a_halfExtent.y, a_halfExtent.z };

	// BoxのShape設定を作成
	JPH::BoxShapeSettings l_boxShapeSettings{ l_halfExtent };

	// ShapeSettingsから実際のShapeを作成する
	const auto& l_shapeResult = l_boxShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "BoxBody用のBoxShapeの作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	// ボックスボディの設定を作成
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													Utility::DirectXMathVector3ToJoltVec3(a_worldPosition),
												    JPH::Quat::sIdentity(),
												    JPH::EMotionType::Static,
												    a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// ボディの作成
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::DontActivate);

	FWK_ASSERT_RETURN_VALUE_IF(l_bodyID.IsInvalid(), "BoxBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = true;

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticCapsuleBody(const PhysicsLayerSetting&		 a_physicsLayerSetting, 
																						 const TypeAlias::Math::Vector3& a_worldPosition, 
																						 const float					 a_halfHeightOfCylinder,
																						 const float				     a_radius, 
																							   JPH::PhysicsSystem&		 a_physicsSystem) const
{
	// Capsuleは円柱部分 + 上下の半球でできている
	// a_halfHeightOfCylinderは円柱部分の半分の高さ
	// JoltではhalfHeightOfCylinder == 0の場合、SphereShapeとして作られるが
	// この関数はカプセル判定を作る関数なので0以下は失敗扱いとする
	FWK_ASSERT_RETURN_VALUE_IF(a_halfHeightOfCylinder <= k_minValidCapsuleHalfHeightOfCylinder, "CapsuleBodyのHalfHeightOfCylinderが0以下のため、作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(a_radius <= k_minValidCapsuleRadius,                             "CapsuleBodyのRadiusが0以下のため、作成に失敗しました。",               {});

	JPH::CapsuleShapeSettings l_capsuleShapeSettings{ a_halfHeightOfCylinder, a_radius };

	const auto& l_shapeResult = l_capsuleShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "CapsuleBody用のCapsuleShape作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													Utility::DirectXMathVector3ToJoltVec3(a_worldPosition),
													JPH::Quat::sIdentity(),
													JPH::EMotionType::Static,
													a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::DontActivate);

	FWK_ASSERT_RETURN_VALUE_IF(l_bodyID.IsInvalid(), "CapsuleBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = true;

	return l_bodyHandle;
}