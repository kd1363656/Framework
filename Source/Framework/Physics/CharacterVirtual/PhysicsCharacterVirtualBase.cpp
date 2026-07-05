#include "PhysicsCharacterVirtualBase.h"

FWK::Physics::PhysicsCharacterVirtualBase::PhysicsCharacterVirtualBase() :
    m_characterVirtual(nullptr),

    m_createWorldPosition(TypeAlias::Math::Vector3::Zero),

    m_extendedUpdateSettings(),

    m_capsuleHalfHeightOfCylinder(Constant::k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder),
    m_capsuleRadius              (Constant::k_defaultCharacterVirtualCapsuleRadius),
    m_maxSlopeAngleRadians       (Constant::k_defaultCharacterVirtualMaxSlopeAngleRadians),

    m_isEnhancedInternalEdgeRemovealDisabled(false)
{}
FWK::Physics::PhysicsCharacterVirtualBase::~PhysicsCharacterVirtualBase()
{
    ReleaseCharacterVirtual();
}

bool FWK::Physics::PhysicsCharacterVirtualBase::CreeateCharacterVirtual()
{
    FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtual,                                                                          "CharacterVirtualが既に作成されています。",                                         false);
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleHalfHeightOfCylinder <= Constant::k_minCharacterVirtualCapsuleHalfHeightOfCylinder, "CharacterVirtualのCapsuleHalfHeightOfCylinderが0以下のため、作成に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleRadius <= Constant:: k_minCharacterVirtualCapsuleRadius,                            "CharacterVirtualのCapsuleRadiusが0以下のため、作成に失敗しました。",               false);

	FWK_ASSERT_RETURN_VALUE_IF(m_maxSlopeAngleRadians < Constant::k_minCharacterVirtualMaxSlopeAngleRadians ||
  		                       m_maxSlopeAngleRadians > Constant::k_maxCharacterVirtualMaxSlopeAngleRadians, 
                               "CharacterVirtualのMaxSlopeAngleが0度から90度の範囲外のため、作成に失敗しました。",
                               false);

    const auto l_shape = CreateShape();

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "CharacterVirtual用CapsuleShapeが無効なため、作成に失敗しました。", false);

    auto& l_physicsManager = PhysicsManager::GetInstance               ();
    auto& l_physicsSystem = l_physicsManager.GetMutableREFPhysicsSystem();

    JPH::CharacterVirtualSettings l_characterVirtualSettings = {};

    // CharacterVirtualで使用する形状
    l_characterVirtualSettings.mShape = l_shape;

    // CharacterVirtual使用時の阪などでずり落ちるかどうかの角度
    l_characterVirtualSettings.mMaxSlopeAngle = m_maxSlopeAngleRadians;


    l_characterVirtualSettings.mEnhancedInternalEdgeRemoval = !m_isEnhancedInternalEdgeRemovealDisabled;

    return false;
}

bool FWK::Physics::PhysicsCharacterVirtualBase::RecreateCharacterVirtual()
{
    return false;
}

void FWK::Physics::PhysicsCharacterVirtualBase::ReleaseCharacterVirtual()
{
    
}

void FWK::Physics::PhysicsCharacterVirtualBase::Update(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime)
{

}

void FWK::Physics::PhysicsCharacterVirtualBase::DrawDebug(const JPH::ColorArg a_color) const
{

}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualBase::FetchVALWorldPosition() const
{
    return TypeAlias::Math::Vector3();
}
FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualBase::FetchVALLinearVelocity() const
{
    return TypeAlias::Math::Vector3();
}

bool FWK::Physics::PhysicsCharacterVirtualBase::FetchVALIsOnGround() const
{
    return false;
}

JPH::Vec3 FWK::Physics::PhysicsCharacterVirtualBase::CalculateLinearVelocity(const JPH::Vec3&                                 a_physicsGravity, 
                                                                             const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, 
                                                                             const float                                      a_deltaTime, 
                                                                                   JPH::CharacterVirtual&                     a_characterVirtual)
{
    return JPH::Vec3();
}

void FWK::Physics::PhysicsCharacterVirtualBase::ApplyExtendedUpdateSettings(const JPH::CharacterVirtual& a_characterVirtual, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdataSettings)
{}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsCharacterVirtualBase::CreateShape() const
{
    return JPH::RefConst<JPH::Shape>();
}

bool FWK::Physics::PhysicsCharacterVirtualBase::ApplyShapeChange()
{
    return false;
}
