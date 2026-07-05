#include "PhysicsCharacterVirtualBase.h"

FWK::Physics::PhysicsCharacterVirtualBase::PhysicsCharacterVirtualBase() :
    m_characterVirtual(nullptr),

    m_createWorldPosition(TypeAlias::Math::Vector3::Zero),

    m_extendedUpdateSettings(),

    m_capsuleHalfHeightOfCylinder(Constant::k_defaultCharacterVirtualCapsuleHalfHeightOfCylinder),
    m_capsuleRadius              (Constant::k_defaultCharacterVirtualCapsuleRadius),
    m_maxSlopeAngleRadians       (Constant::k_defaultCharacterVirtualMaxSlopeAngleRadians),

    m_isEnhancedInternalEdgeRemovalDisabled(false)
{}
FWK::Physics::PhysicsCharacterVirtualBase::~PhysicsCharacterVirtualBase()
{
    ReleaseCharacterVirtual();
}

bool FWK::Physics::PhysicsCharacterVirtualBase::CreateCharacterVirtual()
{
    FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtual,                                                                          "CharacterVirtualが既に作成されています。",                                         false);
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleHalfHeightOfCylinder <= Constant::k_minCharacterVirtualCapsuleHalfHeightOfCylinder, "CharacterVirtualのCapsuleHalfHeightOfCylinderが0以下のため、作成に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleRadius <= Constant:: k_minCharacterVirtualCapsuleRadius,                            "CharacterVirtualのCapsuleRadiusが0以下のため、作成に失敗しました。",               false);

	FWK_ASSERT_RETURN_VALUE_IF(m_maxSlopeAngleRadians < Constant::k_minCharacterVirtualMaxSlopeAngleRadians ||
  		                       m_maxSlopeAngleRadians > Constant::k_maxCharacterVirtualMaxSlopeAngleRadians, 
                               "CharacterVirtualのMaxSlopeAngleが0度から90度の範囲外のため、作成に失敗しました。",
                               false);

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
    l_characterVirtualSettings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -m_capsuleRadius);

    l_characterVirtualSettings.mInnerBodyShape = nullptr;

    // JPH::CharacterVirtual(上で設定したCharacterVirtualSettings、
    //                       CharacterVirtualの初期ワールド座標、
    //                       Charactervirtualの初期回転、
    //                       アプリケーション側で自由に使用できる64bitのUserData、
    //                       CharacterVirtualが床や壁を検索するときに使用する);
    m_characterVirtual = new JPH::CharacterVirtual(&l_characterVirtualSettings,
                                                   Utility::DirectXMathVector3ToJoltRVec3(m_createWorldPosition),
                                                   JPH::Quat::sIdentity(),
                                                   k_defaultUserData,
                                                   &l_physicsSystem);

    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "Jolt側CharacterVirtualの作成に失敗しました。", false);
    
    // ExtendUpdateSettingsをJoltの標準値へ戻す
    m_extendedUpdateSettings = {};

    ApplyExtendedUpdateSettings(*m_characterVirtual, m_extendedUpdateSettings);

    return false;
}

bool FWK::Physics::PhysicsCharacterVirtualBase::RecreateCharacterVirtual()
{
    return false;
}

void FWK::Physics::PhysicsCharacterVirtualBase::ReleaseCharacterVirtual()
{
    // JPH::Refをnullptrへ戻すことで、このクラスが持っている参照カウントを解放する
    m_characterVirtual = nullptr;

    // 前回のCharacterVirtual用設定を壊さないように初期化
    m_extendedUpdateSettings = {};
}

void FWK::Physics::PhysicsCharacterVirtualBase::Update(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime)
{
    FWK_ASSERT_RETURN_IF(!m_characterVirtual,                                                 "CharacterVirtualが作成されていないため、更新に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_deltaTime <= Constant::k_minCharacterVirtualDeltaTime,             "DeltaTimeが0以下のため、CharacterVirtualの更新に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_updateData.m_jumpSpeed < Constant::k_minCharacterVirtualJumpSpeed, "JumpSpeedが0未満のため、CharacterVirtualの更新に失敗しました。");

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
    const auto l_nextLinerVelocity = CalculateLinearVelocity(l_physicsGravity,
                                                             a_updateData,
                                                             a_deltaTime,
                                                             *m_characterVirtual);

    m_characterVirtual->SetLinearVelocity(l_nextLinerVelocity);

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

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualBase::FetchVALWorldPosition() const
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、ワールド座標取得に失敗しました。", {});

    return Utility::JoltVec3ToDirectXMathVector3(m_characterVirtual->GetPosition());
}
FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualBase::FetchVALLinearVelocity() const
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、速度取得に失敗しました。", {});

    return Utility::JoltRVec3ToDirectXMathVector3(m_characterVirtual->GetLinearVelocity());
}

bool FWK::Physics::PhysicsCharacterVirtualBase::FetchVALIsOnGround() const
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_characterVirtual, "CharacterVirtualが作成されていないため、接地状態取得に失敗しました。", false);

    return m_characterVirtual->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
}

void FWK::Physics::PhysicsCharacterVirtualBase::SetWorldCreatePosition(const TypeAlias::Math::Vector3 a_set)
{
    m_createWorldPosition = a_set;
}

JPH::Vec3 FWK::Physics::PhysicsCharacterVirtualBase::CalculateLinearVelocity(const JPH::Vec3&                                 a_physicsGravity, 
                                                                             const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, 
                                                                             const float                                      a_deltaTime, 
                                                                                   JPH::CharacterVirtual&                     a_characterVirtual)
{
    return JPH::Vec3();
}

void FWK::Physics::PhysicsCharacterVirtualBase::ApplyExtendedUpdateSettings(const JPH::CharacterVirtual& a_characterVirtual, JPH::CharacterVirtual::ExtendedUpdateSettings& a_extendedUpdateSettings)
{

}

JPH::RefConst<JPH::Shape> FWK::Physics::PhysicsCharacterVirtualBase::CreateShape() const
{
    FWK_ASSERT_RETURN_VALUE_IF(m_capsuleHalfHeightOfCylinder <= Constant::k_minCharacterVirtualCapsuleHalfHeightOfCylinder, "CapsuleHalfHeightOfCylinderが0以下のため、Shapeの作成に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(m_capsuleRadius               <= Constant::k_minCharacterVirtualCapsuleRadius,               "CapsuleRadiusが0以下のため、Shapeの作成に失敗しました。", {});

    const JPH::CapsuleShapeSettings l_capsuleShapeSettings = { m_capsuleHalfHeightOfCylinder, m_capsuleRadius };

    const auto& l_shapeResult = l_capsuleShapeSettings.Create();

    FWK_ASSERT_RETURN_VALUE_IF(l_shapeResult.HasError(), "CharacterVirtual用CapsuleShapeの作成に失敗しました。", {});

    return l_shapeResult.Get();
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

    // 新しいShapgeへ交換した直後に、周囲のBodyへどの程度めり込んでいても許容するのか
    const float l_maxPenetrationDepth = Constant::k_characterVirtualShapeChangePenetrationSlopScale * l_physicsSystem.GetPhysicsSettings().mPenetrationSlop;
    
    // JPH::CharacterVirtual::SetShape(交換後に使用する新しいShape、
    //                                 Shape交換後に許容する最大貫通深度、
    //                                 BroadPhase深度対策を決めるFilter、
    //                                 衝突可能なObjectLayerを決めるFilter、
    //                                 特定Bodyの除外フィルター、
    //                                 特定SubShapeの除外Filter、
    //                                 Shape交換時の衝突確認で使用する一時Allocator);
    const bool l_isShapeChanged = m_characterVirtual->SetShape(l_shape.GetPtr(),
                                                               l_maxPenetrationDepth,
                                                               l_broadPhaseLayerFilter,
                                                               l_objectLayerFilter,
                                                               l_bodyFilter,
                                                               l_shapeFilter,
                                                               *l_tempAllocator);

    if (!l_isShapeChanged) { return false; }

    return true;
}
