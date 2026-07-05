#include "PhysicsCharacterVirtualUnaffectedByGravity.h"

JPH::Vec3 FWK::Physics::PhysicsCharacterVirtualUnaffectedByGravity::CalculateLinearVelocity(const JPH::Vec3&, 
																	                        const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, 
	                                                                                        const float, 
	                                                                                              JPH::CharacterVirtual&)
{
	return Utility::DirectXMathVector3ToJoltVec3(a_updateData.m_desiredVelocity);
}

JPH::Vec3 FWK::Physics::PhysicsCharacterVirtualUnaffectedByGravity::FetchVALUpdateGravity(const JPH::Vec3&) const
{
	// ExtendedUpdateにも重力を渡さない
	return JPH::Vec3::sZero();
}

void FWK::Physics::PhysicsCharacterVirtualUnaffectedByGravity::ApplyExtendedUpdateSettings(const JPH::CharacterVirtual&, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdateSettings) const
{
	// 重力を受けない自由移動CharacterVirtualを、
	// 床方向へ自動的に引き寄せないようにする。
	a_extendedUpdateSettings.mStickToFloorStepDown = JPH::Vec3::sZero();

	// 空中を自由移動しているときに、
	// 階段昇降処理が実行されないようにする。
	a_extendedUpdateSettings.mWalkStairsStepUp = JPH::Vec3::sZero();
}