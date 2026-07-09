#include "PhysicsCharacterVirtualAffectedByGravity.h"

JPH::Vec3 FWK::Physics::PhysicsCharacterVirtualAffectedByGravity::CalculateLinearVelocity(const JPH::Vec3&                                 a_physicsGravity, 
                                                                                          const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, 
                                                                                          const float                                      a_deltaTime, 
                                                                                                JPH::CharacterVirtual&                     a_characterVirtual)
{
    // CharacterVirtualが載っている床の速度を更新する。
    // 動く床に乗った場合などでもCharacterVirtualも床へ追従できる
    a_characterVirtual.UpdateGroundVelocity();

    const auto& l_up                    = a_characterVirtual.GetUp            ();
    const auto& l_currentLinearVelocity = a_characterVirtual.GetLinearVelocity();
    const auto& l_groundVelocity        = a_characterVirtual.GetGroundVelocity();

    // 現在速度からCharacterVirtualの上下方向成分だけを取得する
    // 水平入力速度は毎フレームa_updateDataから設定し直す
    const auto& l_currentVerticalVelocity = l_currentLinearVelocity.Dot(l_up) * l_up;

    // 床に対してCharacterVirtualが上方向へ離れようとしているかを調べる
    // この値が小さい場合は、床方向へ移動中または由香とほぼ同じ速度である
    const float l_verticalVelocityRelativeToGround = (l_currentVerticalVelocity - l_groundVelocity).Dot(l_up);
    const bool  l_isMovigTowardsGround             = l_verticalVelocityRelativeToGround < PhysicsCharacterVirtualBase::k_characterVirtualMovingTowardsGroundTolerance;

    JPH::Vec3 l_nextLinearVelocity = l_currentVerticalVelocity;

    if (a_characterVirtual.GetGroundState() == JPH::CharacterBase::EGroundState::OnGround &&
        l_isMovigTowardsGround)
    {
        // 接地中は由香の速度を引き継ぐ、
        // StaticBodyなら基本的に0、動く床ならその移動速度になる
        l_nextLinearVelocity = l_groundVelocity;

        // ジャンプ申請を送っているなら受取、ジャンプスピード分加算
        if (a_updateData.m_isJumpRequested)
        {
            // CharacterVirtualの上方向へジャンプ速度を加える
            l_nextLinearVelocity += l_up * a_updateData.m_jumpSpeed;
        }
    }

    // CharacterVirtual自身へ作用する重力波、
    // ExtendedUpdateではなくLinearVelocityへ加算する必要がある
    l_nextLinearVelocity += a_physicsGravity * a_deltaTime;

    const auto& l_desiredVelocity           = Utility::DirectXMathVector3ToJoltVec3              (a_updateData.m_desiredVelocity);
    const auto& l_desiredHorizontalVelocity = l_desiredVelocity - l_desiredVelocity.Dot(l_up) * l_up;

    // 歩行不可能な急斜面へ入り込む方向の速度を取り除いてから、
    // 水平方向の入力速度として加算する。
    l_nextLinearVelocity += a_characterVirtual.CancelVelocityTowardsSteepSlopes(l_desiredHorizontalVelocity);

    return l_nextLinearVelocity;
}

JPH::Vec3 FWK::Physics::PhysicsCharacterVirtualAffectedByGravity::FetchVALUpdateGravity(const JPH::Vec3& a_physicsGravity) const
{
    return a_physicsGravity;
}

void FWK::Physics::PhysicsCharacterVirtualAffectedByGravity::ApplyExtendedUpdateSettings(const JPH::CharacterVirtual& a_characterVirtual, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdateSettings) const
{
    const auto& l_up = a_characterVirtual.GetUp();

    // 床吸着距離の長さはJolt標準値を使用し
    // 方向だけをCharacterVirtualの下方向へ合わせる
    a_extendedUpdateSettings.mStickToFloorStepDown = -l_up * a_extendedUpdateSettings.mStickToFloorStepDown.Length();
    
    // 現在設定されている階段昇降高さを維持したまま、
    // 方向だけをCharacterVirtualの上方向へ合わせる
    a_extendedUpdateSettings.mWalkStairsStepUp = l_up * a_extendedUpdateSettings.mWalkStairsStepUp.Length();
}