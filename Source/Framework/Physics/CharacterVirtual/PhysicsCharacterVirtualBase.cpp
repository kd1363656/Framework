#include "PhysicsCharacterVirtualBase.h"

FWK::Physics::PhysicsCharacterVirtualBase::PhysicsCharacterVirtualBase() :
    m_characterVirtual(nullptr),

    m_extendedUpdateSettings(),

    m_capsuleHalfHeightOfCylinder(k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder),
    m_capsuleRadius              (k_defaultCharacterVirtualCapsuleRadius),
    m_maxSlopeAngleRadians       (k_defaultCharacterVirtualMaxSlopeAngleRadians),

    m_isEnhancedInternalEdgeRemovalDisabled(false)
{}
FWK::Physics::PhysicsCharacterVirtualBase::~PhysicsCharacterVirtualBase()
{
    ReleaseCharacterVirtual();
}

bool FWK::Physics::PhysicsCharacterVirtualBase::CreateCharacterVirtual(const TypeAlias::Math::Quaternion& a_worldRotation, const TypeAlias::Math::Vector3& a_worldPosition)
{
    FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtual,                                                                "CharacterVirtualが既に作成されています。",                                         false);
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleHalfHeightOfCylinder <= k_minCharacterVirtualCapsuleHalfHeightOfCylinder, "CharacterVirtualのCapsuleHalfHeightOfCylinderが0以下のため、作成に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleRadius <= k_minCharacterVirtualCapsuleRadius,                             "CharacterVirtualのCapsuleRadiusが0以下のため、作成に失敗しました。",               false);

	FWK_ASSERT_RETURN_VALUE_IF(m_maxSlopeAngleRadians < k_minCharacterVirtualMaxSlopeAngleRadians ||
  		                       m_maxSlopeAngleRadians > k_maxCharacterVirtualMaxSlopeAngleRadians, 
                               "CharacterVirtualのMaxSlopeAngleが0度から90度の範囲外のため、作成に失敗しました。",
                               false);

    JPH::Quat l_physicsWorldRotation = { a_worldRotation.x,
										 a_worldRotation.y,
										 a_worldRotation.z,
										 a_worldRotation.w };

	FWK_ASSERT_RETURN_VALUE_IF(l_physicsWorldRotation.IsNaN() ||
		                       l_physicsWorldRotation.LengthSq() <= std::numeric_limits<float>::epsilon(),
                               "CharacterVirtualのWorldRotationが無効なため、作成に失敗しました。",
		                       false);

    l_physicsWorldRotation = l_physicsWorldRotation.Normalized();

    // 保存されているCapsuleの高さと半径から、
    // CharacterVirtualが衝突判定に使用するShapeを作成する
    const auto l_shape = CreateShape();

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "CharacterVirtual用CapsuleShapeが無効なため、作成に失敗しました。", false);

    auto& l_physicsManager = PhysicsManager::GetInstance               ();
    auto& l_physicsSystem = l_physicsManager.GetMutableREFPhysicsSystem();

    JPH::CharacterVirtualSettings l_characterVirtualSettings = {};

    // CharacterVirtualにとっての上方向
    // +Yが上方向、シェ面角度、床判定、ジャンプ方向などの基準になる
    l_characterVirtualSettings.mUp = JPH::Vec3::sAxisY();

    // ChacacterVirtualSettingsが衝突判定に使用する形状
    l_characterVirtualSettings.mShape = l_shape;

    // CharacterVirtualが歩行可能な斜面の最大角度
    // 接触面の角度がこの値を超えた場合、Joltはその面を急すぎる斜面として扱う
    l_characterVirtualSettings.mMaxSlopeAngle = m_maxSlopeAngleRadians;

    // MeshShapeなどを構成する三角形同士の教会で
    // ChacacterVirtualが不要に引っかかる現象を軽減する
    l_characterVirtualSettings.mEnhancedInternalEdgeRemoval = !m_isEnhancedInternalEdgeRemovalDisabled;
    
    // Charactervirtualのどの範囲に接触した面を、Charactervirtualを支える床として扱うかを設定する平面
    l_characterVirtualSettings.mSupportingVolume = JPH::Plane{ JPH::Vec3::sAxisY(), -m_capsuleRadius };

    l_characterVirtualSettings.mInnerBodyShape = nullptr;

    // JPH::CharacterVirtual(上で設定したCharacterVirtualSettings、
    //                       CharacterVirtualの初期ワールド座標、
    //                       Charactervirtualの初期回転、
    //                       アプリケーション側で自由に使用できる64bitのUserData、
    //                       CharacterVirtualが床や壁を検索するときに使用する);
    m_characterVirtual = new PhysicsCharacterVirtualInstance{ &l_characterVirtualSettings,
                                                              Utility::DirectXMathVector3ToJoltRVec3(a_worldPosition),
                                                              l_physicsWorldRotation,
                                                              k_defaultUserData,
                                                              &l_physicsSystem };

    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "Jolt側CharacterVirtualの作成に失敗しました。", false);
    
    // ExtendUpdateSettingsをJoltの標準値へ戻す
    m_extendedUpdateSettings = {};

    ApplyExtendedUpdateSettings(*m_characterVirtual, m_extendedUpdateSettings);

    return true;
}

void FWK::Physics::PhysicsCharacterVirtualBase::Update(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime)
{
    FWK_ASSERT_RETURN_IF(!m_characterVirtual,                                       "CharacterVirtualが作成されていないため、更新に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_deltaTime <= k_minCharacterVirtualDeltaTime,             "DeltaTimeが0以下のため、CharacterVirtualの更新に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_updateData.m_jumpSpeed < k_minCharacterVirtualJumpSpeed, "JumpSpeedが0未満のため、CharacterVirtualの更新に失敗しました。");

    const auto& l_physicsManager = PhysicsManager::GetInstance();

    const auto& l_tempAllocator       = l_physicsManager.GetREFTempAllocator      ();
    const auto& l_physicsLayerSetting = l_physicsManager.GetREFPhysicsLayerSetting();

    const auto& l_physicsSystem = l_physicsManager.GetREFPhysicsSystem();

    FWK_ASSERT_RETURN_IF(!l_tempAllocator,       "TempAllocatorが無効なため、CharacterVirtualの更新に失敗しました。");
    FWK_ASSERT_RETURN_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、CharacterVirtualの更新に失敗しました。");

    // CharacterVirtualの衝突Queryで使用するObjectLayerへ登録するわけはなく
    // 「どのLayerを探索してよいか」を判断するために使用する
    const auto l_characterObjectLayer = l_physicsLayerSetting->FetchVALObjectLayer(Enum::PhysicsObjectLayerType::CharacterObject);

    // BroadPhaseのどの領域をCharacterVirtualが探索するかを決めるFilter
    const auto l_broadPhaseLayerFilter = l_physicsSystem.GetDefaultBroadPhaseLayerFilter(l_characterObjectLayer);

    // どのObjectLayerと衝突できるか決めるFilter
    const auto& l_objectLayerFilter = l_physicsSystem.GetDefaultLayerFilter(l_characterObjectLayer);

    // 特定のBodyだけを除外したい場合に使用するFilter
    const JPH::BodyFilter l_bodyFilter = {};

    // 特定SubShapeを除外したい場合に使用するフィルター
    const JPH::ShapeFilter l_shapeFilter = {};

    const auto l_physicsGravity = l_physicsSystem.GetGravity();

    // 実際にSetLinerVelocityへ渡す次の速度を、派生クラスで計算する
    const auto& l_nextLinearVelocity = CalculateLinearVelocity(l_physicsGravity,
                                                               a_updateData,
                                                               a_deltaTime,
                                                               *m_characterVirtual);

    m_characterVirtual->SetLinearVelocity(l_nextLinearVelocity);
    
    // CharacterVirtualは現在のLinearVelocityをもとにExtendedUpdate()で移動する
    ApplyExtendedUpdateSettings(*m_characterVirtual, m_extendedUpdateSettings);

    // ExtendedUpdate()へ渡す重力を派生クラスから取得する
    const auto& l_updateGravity = FetchVALUpdateGravity(l_physicsGravity);

    // CjaracterVirtualの移動と衝突判定を実行する
    m_characterVirtual->ExtendedUpdate(a_deltaTime,
                                       l_updateGravity,
                                       m_extendedUpdateSettings,
                                       l_broadPhaseLayerFilter,
                                       l_objectLayerFilter,
                                       l_bodyFilter,
                                       l_shapeFilter, 
                                       *l_tempAllocator);
}

void FWK::Physics::PhysicsCharacterVirtualBase::DrawDebug(const JPH::ColorArg a_color) const
{
    if (!m_characterVirtual) { return; }

    const auto& l_physicsManager = PhysicsManager::GetInstance();

    if (l_physicsManager.GetVALIsDisableDebugDraw()) { return; }

    const auto l_debugRenderer = l_physicsManager.GetVALDebugRenderer().lock();

    FWK_ASSERT_RETURN_IF(!l_debugRenderer, "PhysicsDebugRendererが無効なため、CharacterVirtualのデバッグ描画に失敗しました。");

    const auto* const l_shape = m_characterVirtual->GetShape();

    FWK_ASSERT_RETURN_IF(!l_shape, "CharacterVirtualのShapeが無効なため、デバック描画に失敗しました。");

    // JPH::Shape::Draw(Joltの描画命令をフレームワーク側のVertexへ変換するRenderer、
    //                  CharacterVirtualのShape中心をワールド座標へ変換する行列、
    //                  Shapeへ適用するスケール、
    //                  デバック表示で使用する色、
    //                  Material個夕食を使用するか、
    //                  ワイヤーフレームで描画するか);
    l_shape->Draw(l_debugRenderer.get(),
                  m_characterVirtual->GetCenterOfMassTransform(),
                  JPH::Vec3::sOne(),
                  a_color,
                  false,
                  true);


}

void FWK::Physics::PhysicsCharacterVirtualBase::ReleaseCharacterVirtual()
{
    // JPH::Refをnullptrへ戻すことで、このクラスが持っている参照カウントを解放する
    m_characterVirtual = nullptr;

    // 前回のCharacterVirtual用設定を壊さないように初期化
    m_extendedUpdateSettings = {};
}

bool FWK::Physics::PhysicsCharacterVirtualBase::ApplyWorldTransform(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Quaternion& a_worldRotation)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、ワールド座標の反映に失敗しました。", false);

    JPH::Quat l_physicsWorldRotation = { a_worldRotation.x,
                                         a_worldRotation.y,
                                         a_worldRotation.z,
                                         a_worldRotation.w };

    FWK_ASSERT_RETURN_VALUE_IF(l_physicsWorldRotation.IsNaN() ||
                               l_physicsWorldRotation.LengthSq() <= std::numeric_limits<float>::epsilon(), 
                               "CharacterVirtualのWorldRotationが無効なため、WorldTransformの反映に失敗しました。", 
                                false);

    l_physicsWorldRotation = l_physicsWorldRotation.Normalized();

    // 位置とオフセットが計算された状態のものをセットする想定
    m_characterVirtual->SetPosition(Utility::DirectXMathVector3ToJoltRVec3(a_worldPosition));
    m_characterVirtual->SetRotation(l_physicsWorldRotation);

    return true;
}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualBase::FetchVALWorldPosition() const
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、ワールド座標取得に失敗しました。", {});

    return Utility::JoltRVec3ToDirectXMathVector3(m_characterVirtual->GetPosition());
}
FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualBase::FetchVALLinearVelocity() const
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、速度取得に失敗しました。", {});

    return Utility::JoltVec3ToDirectXMathVector3(m_characterVirtual->GetLinearVelocity());
}

bool FWK::Physics::PhysicsCharacterVirtualBase::FetchVALIsOnGround() const
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、接地状態取得に失敗しました。", false);

    return m_characterVirtual->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsCharacterVirtualBase::CreateShape() const
{
    FWK_ASSERT_RETURN_VALUE_IF(m_capsuleHalfHeightOfCylinder <= k_minCharacterVirtualCapsuleHalfHeightOfCylinder, "CapsuleHalfHeightOfCylinderが0以下のため、Shapeの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleRadius               <= k_minCharacterVirtualCapsuleRadius,               "CapsuleRadiusが0以下のため、Shapeの作成に失敗しました。", {});

    const JPH::CapsuleShapeSettings l_capsuleShapeSettings = { m_capsuleHalfHeightOfCylinder, m_capsuleRadius };

    const auto& l_capsuleShapeResult = l_capsuleShapeSettings.Create();

    FWK_ASSERT_RETURN_VALUE_IF(l_capsuleShapeResult.HasError(), "CharacterVirtual用CapsuleShapeの作成に失敗しました。", {});

    const auto& l_capsuleShape = l_capsuleShapeResult.Get();

    // CapsuleShapeは中心がローカル原点にあるため、
    // Chacactervirtualの座標を足元として扱えるように上方向へずらす
    const float l_capsuleCenterOffsetY = m_capsuleHalfHeightOfCylinder + m_capsuleRadius;

    const JPH::RotatedTranslatedShapeSettings l_characterShapeSettings = { JPH::Vec3(JPH::Vec3::sZero().GetX(), l_capsuleCenterOffsetY, JPH::Vec3::sZero().GetZ()),
                                                                           JPH::Quat::sIdentity(),
                                                                           l_capsuleShape.GetPtr() };

    const auto& l_characterShapeResult = l_characterShapeSettings.Create();

    FWK_ASSERT_RETURN_VALUE_IF(l_characterShapeResult.HasError(), "CharacterVirtual用の足元原点CapsuleShapeの作成に失敗しました。", {});

    return l_characterShapeResult.Get();
}

bool FWK::Physics::PhysicsCharacterVirtualBase::ApplyShapeChange()
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、Shape変更に失敗しました。", false);

    const auto l_shape = CreateShape();

    FWK_ASSERT_RETURN_VALUE_IF(!l_shape, "変更後のCharacterVirtual用CapsuleShapeが無効です。", false);

    const auto& l_physicsManager = PhysicsManager::GetInstance();

    const auto& l_tempAllocator       = l_physicsManager.GetREFTempAllocator      ();
    const auto& l_physicsLayerSetting = l_physicsManager.GetREFPhysicsLayerSetting();
    const auto& l_physicsSystem       = l_physicsManager.GetREFPhysicsSystem      ();
   
    FWK_ASSERT_RETURN_VALUE_IF(!l_tempAllocator,       "TempAllocatorが無効なため、Shape変更に失敗しました。",       false);
    FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、Shape変更に失敗しました。", false);

    const auto& l_characterObjectLayer  = l_physicsLayerSetting->FetchVALObjectLayer     (Enum::PhysicsObjectLayerType::CharacterObject);
    const auto& l_broadPhaseLayerFilter = l_physicsSystem.GetDefaultBroadPhaseLayerFilter(l_characterObjectLayer);
    const auto& l_objectLayerFilter     = l_physicsSystem.GetDefaultLayerFilter          (l_characterObjectLayer);

    const JPH::BodyFilter  l_bodyFilter  = {};
    const JPH::ShapeFilter l_shapeFilter = {};

    // 変更がなければreturn
    // l_maxPenetrationDepthは新しいShapgeへ交換した直後に、周囲のBodyへどの程度めり込んでいても許容するのか
    // JPH::CharacterVirtual::SetShape(交換後に使用する新しいShape、
    //                                 Shape交換後に許容する最大貫通深度、
    //                                 BroadPhase深度対策を決めるFilter、
    //                                 衝突可能なObjectLayerを決めるFilter、
    //                                 特定Bodyの除外フィルター、
    //                                 特定SubShapeの除外Filter、
    //                                 Shape交換時の衝突確認で使用する一時Allocator);
    if (const float l_maxPenetrationDepth = k_characterVirtualShapeChangePenetrationSlopScale * l_physicsSystem.GetPhysicsSettings().mPenetrationSlop;
        !m_characterVirtual->SetShape(l_shape.GetPtr(),
                                     l_maxPenetrationDepth,
                                     l_broadPhaseLayerFilter,
                                     l_objectLayerFilter,
                                     l_bodyFilter,
                                     l_shapeFilter,
                                     *l_tempAllocator))
    {
        return false; 
    }

    // Radiusが変更されると、CharacterVirtualを支える床の判定範囲も変わるため、
    // 新しいRadiusから作成したSupportingVolumeへ更新する。
    m_characterVirtual->SetSupportingVolume(JPH::Plane{ JPH::Vec3::sAxisY(), -m_capsuleRadius });

    return true;
}
