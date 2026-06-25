#include "PhysicsBodyCreator.h"


FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticSphereBody(const PhysicsLayerSetting&	    a_physicsLayerSetting, 
																				        const TypeAlias::Math::Vector3& a_worldPosition,
																				        const float					    a_radius, 
																				        	  JPH::PhysicsSystem&       a_physicsSystem) const
{
	return CreateSphereBody(a_physicsLayerSetting,
						    a_worldPosition,
							JPH::EMotionType::Static,
							JPH::EActivation::DontActivate,
							Enum::PhysicsObjectLayerType::StaticObject,
						    a_radius,
							k_linearCastDisabled,
							a_physicsSystem);
}
FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateDynamicSphereBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
																						 const TypeAlias::Math::Vector3& a_worldPosition, 
																						 const float					 a_radius, 
																							   JPH::PhysicsSystem&       a_physicsSystem) const
{
	return CreateSphereBody(a_physicsLayerSetting,
						    a_worldPosition,
							JPH::EMotionType::Dynamic,
							JPH::EActivation::Activate,
							Enum::PhysicsObjectLayerType::DynamicObject,
						    a_radius,
							k_linearCastEnabled,
							a_physicsSystem);
}


FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting,
																					 const TypeAlias::Math::Vector3& a_worldPosition,
																					 const TypeAlias::Math::Vector3& a_halfExtent, 
																						   JPH::PhysicsSystem&       a_physicsSystem) const
{
	return CreateBoxBody(a_physicsLayerSetting,
					     a_worldPosition,
						 a_halfExtent,
						 JPH::EMotionType::Static,
						 JPH::EActivation::DontActivate,
						 Enum::PhysicsObjectLayerType::StaticObject,
						 k_linearCastDisabled,
						 a_physicsSystem);
}
FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateDynamicBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting, 
																					  const TypeAlias::Math::Vector3& a_worldPosition, 
																					  const TypeAlias::Math::Vector3& a_halfExtent, 
																							JPH::PhysicsSystem&       a_physicsSystem) const
{
		return CreateBoxBody(a_physicsLayerSetting,
					     a_worldPosition,
						 a_halfExtent,
						 JPH::EMotionType::Dynamic,
						 JPH::EActivation::Activate,
						 Enum::PhysicsObjectLayerType::DynamicObject,
						 k_linearCastEnabled,
						 a_physicsSystem);
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticCapsuleBody(const PhysicsLayerSetting&		 a_physicsLayerSetting, 
																						 const TypeAlias::Math::Vector3& a_worldPosition, 
																						 const float					 a_halfHeightOfCylinder,
																						 const float				     a_radius, 
																							   JPH::PhysicsSystem&		 a_physicsSystem) const
{
	return CreateCapsuleBody(a_physicsLayerSetting,
					         a_worldPosition,
						     JPH::EMotionType::Static,
						     JPH::EActivation::DontActivate,
						     Enum::PhysicsObjectLayerType::StaticObject,
						     a_halfHeightOfCylinder,
						     a_radius,
							 k_linearCastDisabled,
						     a_physicsSystem);
}
FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateDynamicCapsuleBody(const PhysicsLayerSetting&      a_physicsLayerSetting, 
																						  const TypeAlias::Math::Vector3& a_worldPosition,
																						  const float					  a_halfHeightOfCylinder, 
																						  const float					  a_radius,		
																								JPH::PhysicsSystem&       a_physicsSystem) const
{
	return CreateCapsuleBody(a_physicsLayerSetting,
					         a_worldPosition,
						     JPH::EMotionType::Dynamic,
						     JPH::EActivation::Activate,
						     Enum::PhysicsObjectLayerType::DynamicObject,
						     a_halfHeightOfCylinder,
						     a_radius,
						     k_linearCastEnabled,
						     a_physicsSystem);
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateSphereBody(const PhysicsLayerSetting&		     a_physicsLayerSetting, 
																				  const TypeAlias::Math::Vector3&    a_worldPosition, 
																				  const JPH::EMotionType		     a_motionType,
																				  const JPH::EActivation		     a_activationType, 
																				  const Enum::PhysicsObjectLayerType a_objectLayerType, 
																				  const float						 a_radius, 
																				  const bool						 a_isLinearCastEnabled, 
																						JPH::PhysicsSystem&			 a_physicsSystem) const
{
	// 半径が0以下のSphereは、当たり判定として成立しない
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_radius <= k_minValidSphereRadius, "SphereBodyのRadiusが0以下のため、作成に失敗しました。", {});

	// 半径がどれぐらいの球を作るのかを伝える設定
	const JPH::SphereShapeSettings l_sphereShapeSettings{ a_radius };

	const auto& l_shapeResult = l_sphereShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "SphereBody用のSphereShapeの作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	// スフィアボディの設定を作成
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													Utility::ConvertToJoltVector3(a_worldPosition),
													JPH::Quat::sIdentity(),
													a_motionType,
													a_physicsLayerSetting.FetchVALObjectLayer(a_objectLayerType) };

	if (a_isLinearCastEnabled)
	{
		// DynamicSphereは速度で移動するため、薄い床や壁をすり抜ける可能性がある、
		// LinearCastを有効にすると、移動後の位置だけでなく移動経路も見てくれる
		l_bodyCreateSettings.mMotionQuality = JPH::EMotionQuality::LinearCast;
	}

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// ボディの作成
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, a_activationType);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "SphereBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateBoxBody(const PhysicsLayerSetting&         a_physicsLayerSetting, 
																			   const TypeAlias::Math::Vector3&    a_worldPosition, 
																			   const TypeAlias::Math::Vector3&    a_halfExtent, 
																			   const JPH::EMotionType             a_motionType, 
																			   const JPH::EActivation             a_activationType, 
																			   const Enum::PhysicsObjectLayerType a_objectLayerType, 
																			   const bool						  a_isLinearCastEnabled,
																				     JPH::PhysicsSystem&		  a_physicsSystem) const
{
	// BoxのHalfExtentは中央から各軸方向への半分の長さ
	// 例えば幅10,高さ2,奥行き4ならHalfExtentはx = 5, y = 1, z = 2
	// どれか一つでも0以下だと、厚みのない無効なBoxになる
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_halfExtent.x <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.y <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.z <= k_minValidBoxHalfExtentLength,
									  "BoxBodyのHalfExtentが0以下のため、作成に失敗しました。",
									  {});

	const JPH::Vec3& l_halfExtent = { a_halfExtent.x, a_halfExtent.y, a_halfExtent.z };

	// BoxのShape設定を作成
	JPH::BoxShapeSettings l_boxShapeSettings{ l_halfExtent };

	// ShapeSettingsから実際のShapeを作成する
	const auto& l_shapeResult = l_boxShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "BoxBody用のBoxShapeの作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	// ボックスボディの設定を作成
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													Utility::ConvertToJoltVector3(a_worldPosition),
												    JPH::Quat::sIdentity(),
												    a_motionType,
												    a_physicsLayerSetting.FetchVALObjectLayer(a_objectLayerType) };

	if (a_isLinearCastEnabled)
	{
		// DynamicBoxは移動中に壁や床をすり抜ける可能性があるため、
		// DynamicBodyのときだけLinearCastを有効化する。
		l_bodyCreateSettings.mMotionQuality = JPH::EMotionQuality::LinearCast;
	}

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// ボディの作成
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, a_activationType);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "BoxBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateCapsuleBody(const PhysicsLayerSetting&         a_physicsLayerSetting, 
																			       const TypeAlias::Math::Vector3&    a_worldPosition, 
																			       const JPH::EMotionType             a_motionType, 
																			       const JPH::EActivation		      a_activationType, 
																			       const Enum::PhysicsObjectLayerType a_objectLayerType, 
																			       const float						  a_halfHeightOfCylinder,
																		           const float						  a_radius,
																			       const bool						  a_isLinearCastEnabled, 
																			       	     JPH::PhysicsSystem&		  a_physicsSystem) const
{
	// Capsuleは円柱部分 + 上下の半球でできている
	// a_halfHeightOfCylinderは円柱部分の半分の高さ
	// JoltではhalfHeightOfCylinder == 0の場合、SphereShapeとして作られるが
	// この関数はカプセル判定を作る関数なので0以下は失敗扱いとする
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_halfHeightOfCylinder <= k_minValidCapsuleHalfHeightOfCylinder, "CapsuleBodyのHalfHeightOfCylinderが0以下のため、作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_radius <= k_minValidCapsuleRadius,							   "CapsuleBodyのRadiusが0以下のため、作成に失敗しました。",			   {});

	JPH::CapsuleShapeSettings l_capsuleShapeSettings{ a_halfHeightOfCylinder, a_radius };

	const auto& l_shapeResult = l_capsuleShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "CapsuleBody用のCapsuleShape作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													Utility::ConvertToJoltVector3(a_worldPosition),
													JPH::Quat::sIdentity(),
													a_motionType,
													a_physicsLayerSetting.FetchVALObjectLayer(a_objectLayerType) };

	if (a_isLinearCastEnabled)
	{
		// DynamicCapsuleはプレイヤーや敵の仮Bodyとして使う可能性が高い
		// 移動中や壁や床を抜けにくくするため、Dynamicの時だけLinearCastを有効化する
		l_bodyCreateSettings.mMotionQuality = JPH::EMotionQuality::LinearCast;
	}

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, a_activationType);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "CapsuleBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

	return l_bodyHandle;
}