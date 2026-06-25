#include "PhysicsBodyCreator.h"


FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticSphereBody(const PhysicsLayerSetting&	    a_physicsLayerSetting, 
																				        const TypeAlias::Math::Vector3& a_worldPosition,
																				        const float					    a_radius, 
																				        	  JPH::PhysicsSystem&       a_physicsSystem) const
{
	// 半径が0以下のSphereは、当たり判定として成立しない
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_radius <= k_minValidSphereRadius, "StaticSphereBodyのRadiusが0以下のため、作成に失敗しました。", {});

	JPH::SphereShapeSettings l_sphereShapeSettings(a_radius);

	const auto l_shapeResult = l_sphereShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "StaticSphereBody用のSphereShapeの作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	// StaticObjectは動かない判定
	// マップ、壁、床、固定オブジェクトなどに使う
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													ConvertToJoltVector3(a_worldPosition),
													JPH::Quat::sIdentity(),
													JPH::EMotionType::Static,
													a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::StaticObject) };

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	// StaticBodyは自分から動かないため、作成直後にActivateしない
	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::DontActivate);

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
	// 半径が0以下のSphereは、当たり判定として成立しない
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_radius <= k_minValidSphereRadius, "DynamicSphereBodyのRadiusが0以下のため、作成に失敗しました。", {});

	// SphereのShape設定を作る
	JPH::SphereShapeSettings l_sphereShapeSettings(a_radius);

	// ShapeSettingsから実際のShapeを作成する
	const auto l_shapeResult = l_sphereShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "DynamicSphereBody用のSphereShapeの作成に失敗しました。", {});

	// 作成済みShapeを取得する
	const auto& l_shape = l_shapeResult.Get();

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

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "DynamicSphereBody用のSphereShapeの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

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
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_halfExtent.x <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.y <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.z <= k_minValidBoxHalfExtentLength,
									  "StaticBoxBodyのHalfExtentが0以下のため、作成に失敗しました。",
									  {});

	const JPH::Vec3 l_halfExtent = ConvertToJoltVector3(a_halfExtent);

	// BoxのShape設定を作成
	JPH::BoxShapeSettings l_boxShapeSettings(l_halfExtent);

	// ShapeSettingsから実際のShapeを作成する
	const auto l_shapeResult = l_boxShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "StaticBoxBody用のBoxShapeの作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

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
FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateDynamicBoxBody(const PhysicsLayerSetting&      a_physicsLayerSetting, 
																					  const TypeAlias::Math::Vector3& a_worldPosition, 
																					  const TypeAlias::Math::Vector3& a_halfExtent, 
																							JPH::PhysicsSystem&       a_physicsSystem) const
{
	// BoxのHalfExtentは中央から各軸方向への半分の長さ
	// どれか一つでも0以下だと、厚みのない無効なBoxになる
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(a_halfExtent.x <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.y <= k_minValidBoxHalfExtentLength ||
									  a_halfExtent.z <= k_minValidBoxHalfExtentLength,
									  "DynamicBoxBodyのHalfExtentが0以下のため、作成に失敗しました。",
									  {});

	const JPH::Vec3 l_halfExtent = ConvertToJoltVector3(a_halfExtent);

	// Jolt側のBox形状設定を作る
	JPH::BoxShapeSettings l_boxShapeSettings(l_halfExtent);

	// ShapeSettingsから実際に物理計算で使うShapeを作る
	const auto l_shapeResult = l_boxShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_shapeResult.HasError(), "DynamicBoxBody用のBoxShape作成に失敗しました。", {});

	const auto& l_shape = l_shapeResult.Get();

	// DynamicObjectは重力や速度によって動くBody
	// 壁や床に当たるとJolt側で衝突解決される。
	JPH::BodyCreationSettings l_bodyCreateSettings{ l_shape,
													ConvertToJoltVector3(a_worldPosition),
													JPH::Quat::sIdentity(),
													JPH::EMotionType::Dynamic,
													a_physicsLayerSetting.FetchVALObjectLayer(Enum::PhysicsObjectLayerType::DynamicObject) };

	// DynamicBodyは移動中にすり抜ける可能性がある。
	// LinearCastにしておくことで、高速移動時も移動経路を見て接触を検出しやすくなる
	l_bodyCreateSettings.mMotionQuality = JPH::EMotionQuality::LinearCast;

	auto& l_bodyInterface = a_physicsSystem.GetBodyInterface();

	const auto l_bodyID = l_bodyInterface.CreateAndAddBody(l_bodyCreateSettings, JPH::EActivation::Activate);

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_bodyID.IsInvalid(), "DynamicBoxBodyの作成に失敗しました。", {});

	Struct::PhysicsBodyHandle l_bodyHandle = {};

	l_bodyHandle.m_bodyID  = l_bodyID;
	l_bodyHandle.m_isValid = !l_bodyID.IsInvalid();

	return l_bodyHandle;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateStaticCapsuleBody(const PhysicsLayerSetting&		 a_physicsLayerSetting, 
																						 const TypeAlias::Math::Vector3& a_worldPosition, 
																						 const float					 a_halfHeightOfCylinder,
																						 const float				     a_radius, 
																							   JPH::PhysicsSystem&		 a_physicsSystem) const
{
	return Struct::PhysicsBodyHandle();
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyCreator::CreateDynamicCapsuleBody(const PhysicsLayerSetting&      a_physicsLayerSetting, 
																						  const TypeAlias::Math::Vector3& a_worldPosition,
																						  const float					  a_halfHeightOfCylinder, 
																						  const float					  a_radius,		
																								JPH::PhysicsSystem&       a_physicsSystem) const
{
	return Struct::PhysicsBodyHandle();
}

JPH::Vec3 FWK::Physics::PhysicsBodyCreator::ConvertToJoltVector3(const TypeAlias::Math::Vector3& a_vector) const
{
	// NowProject側のVector3をJolt側のVec3へ変換する
	return JPH::Vec3{ a_vector.x, a_vector.y, a_vector.z };
}
