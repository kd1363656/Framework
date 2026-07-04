#include "PhysicsCharacterVirtualRegistry.h"

FWK::Physics::PhysicsCharacterVirtualRegistry::PhysicsCharacterVirtualRegistry() : 
	m_characterVirtualRecordList(),

	m_physicsSystem		 (),
	m_physicsLayerSetting(),
	m_tempAllocator      (),

	m_storageIDAllocator(),

	m_isSetup(false)
{}
FWK::Physics::PhysicsCharacterVirtualRegistry::~PhysicsCharacterVirtualRegistry()
{
	// セットアップが完了していなければreturn
	if (!m_isSetup) { return; }

	ReleaseAllCharacterVirtuals();
}

bool FWK::Physics::PhysicsCharacterVirtualRegistry::Setup(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const std::shared_ptr<PhysicsLayerSetting>& a_physicsLayerSetting, const std::shared_ptr<JPH::TempAllocatorImpl>& a_tempAllocator)
{
	FWK_ASSERT_RETURN_VALUE_IF(m_isSetup,              "PhysicsCharacterVirtualRegistryが既に設定されています。",                                false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_physicsSystem,       "PhysicsSystemが無効なため、PhysicsCharacterVirtualRegistryの設定に失敗しました。",       false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_physicsLayerSetting, "PhysicsLayerSettingが無効なため、PhysicsCharacterVirtualRegistryの設定に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_tempAllocator,       "TempAllocatorが無効なため、PhysicsCharacterVirtualRegistryの設定に失敗しました。",       false);

	m_physicsSystem		  = a_physicsSystem;
	m_physicsLayerSetting = a_physicsLayerSetting;
	m_tempAllocator       = a_tempAllocator;

	FWK_ASSERT_RETURN_VALUE_IF(!SetupStorage(), "CharacterVirtual用Storageの設定に失敗しました。", false);

	m_isSetup = true;

	return true;
}

void FWK::Physics::PhysicsCharacterVirtualRegistry::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::Physics::PhysicsCharacterVirtualRegistry::UpdateCharacterVirtual(const TypeAlias::StorageID a_characterVirtualStorageID, const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime)
{
	FWK_ASSERT_RETURN_IF(!m_isSetup,												          "PhysicsCharacterVirtualRegistryが設定されていないため、CharacterVirtualの更新に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_deltaTime <= Constant::k_minCharacterVirtualDeltaTime,			  "DeltaTimeが0以下のため、CharacterVirtualの更新に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_updateData.m_jumpSpeed < Constant::k_minCharacterVirtualJumpSpeed, "JumpSpeedが0未満のため、CharacterVirtualの更新に失敗しました。");

		  auto& l_characterVirtualRecord = m_characterVirtualRecordList[a_characterVirtualStorageID];
	const auto& l_characterVirtual       = l_characterVirtualRecord.m_characterVirtual;

	FWK_ASSERT_RETURN_IF(!l_characterVirtual, "CharacterVirtualが無効のため、CharacterVirtualの更新に失敗しました。");

	UpdateCharacterVirtualRecord(a_updateData, a_deltaTime, l_characterVirtualRecord);
}

nlohmann::json FWK::Physics::PhysicsCharacterVirtualRegistry::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

FWK::TypeAlias::StorageID FWK::Physics::PhysicsCharacterVirtualRegistry::ReleaseCharacterVirtual(const TypeAlias::StorageID a_characterVirtualStorageID)
{
	// 解放失敗時は元のStorageIDを返し、
	// 呼び出し側が有効なIDを失わないようにする。
	FWK_ASSERT_RETURN_VALUE_IF(!m_isSetup,																				"PhysicsCharacterVirtualRegistryが設定されていないため、CharacterVirtualの解放に失敗しました。", Constant::k_invalidStorageID);
	FWK_ASSERT_RETURN_VALUE_IF(a_characterVirtualStorageID == Constant::k_invalidStorageID,								"CharacterVirtualのStorageIDが無効なため、解放に失敗しました。",                                 Constant::k_invalidStorageID);
	FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtualRecordList.size() <= static_cast<size_t>(a_characterVirtualStorageID), "リストのサイズを超えており、解放に失敗しました。",                                              Constant::k_invalidStorageID);
	
	auto& l_characterVirtualRecord = m_characterVirtualRecordList[a_characterVirtualStorageID];

	// JPH::Refをnullptrで戻すことでCharacterVirtual本体を即時解放する
	l_characterVirtualRecord.m_characterVirtual       = nullptr;
	l_characterVirtualRecord.m_extendedUpdateSettings = {};

	// StorageIDをAllocatorに返却する
	m_storageIDAllocator.Release(a_characterVirtualStorageID);

	// 解放成功後に呼び出し側が保持するIDは無効値
	return Constant::k_invalidStorageID;
}

FWK::TypeAlias::StorageID FWK::Physics::PhysicsCharacterVirtualRegistry::CreateCharacterVirtual(const Struct::PhysicsCharacterVirtualCreateSetting& a_createSetting)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isSetup,																								    "PhysicsCharacterVirtualRegistryが設定されていないため、CreateCharacterVirtual処理に失敗しました。。",          Constant::k_invalidStorageID);
	FWK_ASSERT_RETURN_VALUE_IF(a_createSetting.m_capsuleHalfHeightOfCylinder <= Constant::k_minCharacterVirtualCapsuleHalfHeightOfCylinder, "CharacterVirtualのCapsuleHalfHeightOfCylinderが0以下となっており、CreateCharacterVirtual処理に失敗しました。", Constant::k_invalidStorageID);
	FWK_ASSERT_RETURN_VALUE_IF(a_createSetting.m_capsuleRadius               <= Constant::k_minCharacterVirtualCapsuleRadius,               "CharacterVirtualのCapsuleRadiusが0以下となっており、CreateCharacterVirtual処理に失敗しました。",               Constant::k_invalidStorageID);

	FWK_ASSERT_RETURN_VALUE_IF(a_createSetting.m_maxSlopeAngleRadians < Constant::k_minCharacterVirtualMaxSlopeAngleRadians ||
		                       a_createSetting.m_maxSlopeAngleRadians > Constant::k_maxCharacterVirtualMaxSlopeAngleRadians,
		                       "CharacterVirtualのMaxSlopeAngleが0度から90度の範囲外となっており、CreateCharacterVirtual処理に失敗しました。",
		                       Constant::k_invalidStorageID);

	FWK_ASSERT_RETURN_VALUE_IF(a_createSetting.m_characterVirtualType == Enum::PhysicsCharacterVirtualType::Invalid, "CharacterVirtualTypeがInvalidのため、CharacterVirtual処理に失敗しました。", Constant::k_invalidStorageID);

	const auto l_physicsSystem = m_physicsSystem.lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsSystem, "PhysicsSystemが無効なため、CharacterVirtualの作成に失敗しました。", Constant::k_invalidStorageID);

	// カプセルの半径と高さを設定
	JPH::CapsuleShapeSettings l_capsuleShapeSettings = { a_createSetting.m_capsuleHalfHeightOfCylinder, a_createSetting.m_capsuleRadius };

	const auto& l_capsuleShapeResult = l_capsuleShapeSettings.Create();

	FWK_ASSERT_RETURN_VALUE_IF(l_capsuleShapeResult.HasError(), "CharacterVirtual用CapsuleShapeの作成に失敗しました。", Constant::k_invalidStorageID);

	JPH::CharacterVirtualSettings l_characterVirtualSettings = {};

	// 設定したカプセルと傾斜物が何度あれば滑るのかといったデータを渡す
	l_characterVirtualSettings.mShape		                = l_capsuleShapeResult.Get();
	l_characterVirtualSettings.mMaxSlopeAngle               = a_createSetting.m_maxSlopeAngleRadians;
	l_characterVirtualSettings.mEnhancedInternalEdgeRemoval = !a_createSetting.m_isEnhancedInternalEdgeRemovalDisabled;

	// カプセル下側の半球に触れた面を、
	// CharacterVirtualを支える床として判定する
	l_characterVirtualSettings.mSupportingVolume = JPH::Plane{ JPH::Vec3::sAxisY(), -a_createSetting.m_capsuleRadius };

	const auto l_characterVirtualStorageID = m_storageIDAllocator.Allocate();

	FWK_ASSERT_RETURN_VALUE_IF(l_characterVirtualStorageID == Constant::k_invalidStorageID,                                  "CharacterVirtual用StorageIDの割り当てに失敗しており、CreateCharacterVirtual処理に失敗しました。", Constant::k_invalidStorageID);
	FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtualRecordList.size() <= static_cast<std::size_t>(l_characterVirtualStorageID), "リストのサイズを超えており、解放に失敗しました。",	                                                Constant::k_invalidStorageID);

	auto& l_characterVirtualRecord = m_characterVirtualRecordList[l_characterVirtualStorageID];

	// 再利用されたStorageIDのRecordは、
	// 前回の解放時に空になっている必要がある。
	FWK_ASSERT_RETURN_VALUE_IF(l_characterVirtualRecord.m_characterVirtual, "使用中のCharacterVirtualRecordへ新しいCharacterVirtualを登録しようとており、CreateCharacterVirtual処理に失敗しました。", Constant::k_invalidStorageID);

	JPH::Ref<JPH::CharacterVirtual> l_characterVirtual = new JPH::CharacterVirtual(&l_characterVirtualSettings,
																				   Utility::DirectXMathVector3ToJoltRVec3(a_createSetting.m_worldPosition),
																				   JPH::Quat::sIdentity(),
																				   static_cast<JPH::uint64>(l_characterVirtualStorageID),
																				   l_physicsSystem.get());
	
	l_characterVirtualRecord.m_characterVirtual       = l_characterVirtual;
	
	auto& l_extendedUpdateSettings = l_characterVirtualRecord.m_extendedUpdateSettings;

	// 重力がないTypeなら床吸着と階段昇降を無効化
	if (a_createSetting.m_characterVirtualType == Enum::PhysicsCharacterVirtualType::UnaffectedByGravity)
	{
		l_extendedUpdateSettings.mStickToFloorStepDown = JPH::Vec3::sZero();
		l_extendedUpdateSettings.mWalkStairsStepUp     = JPH::Vec3::sZero();
	}
	else
	{
		l_extendedUpdateSettings = {};
	}

	// CharacterVirtualTypeを格納
	l_characterVirtualRecord.m_characterVirtualType = a_createSetting.m_characterVirtualType;

	return l_characterVirtualStorageID;
}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualRegistry::FetchVALCharacterVirtualWorldPosition(const TypeAlias::StorageID a_characterVirtualStorageID) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_characterVirtualStorageID         == Constant::k_invalidStorageID,                     "StorageIDが無効なため、ワールド座標取得に失敗しました。",      {});
	FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtualRecordList.size() <= static_cast<size_t>(a_characterVirtualStorageID), "リストのサイズを超えており、ワールド座標取得に失敗しました。", {});

	const auto& l_characterVirtual = m_characterVirtualRecordList[a_characterVirtualStorageID].m_characterVirtual;

	FWK_ASSERT_RETURN_VALUE_IF(!l_characterVirtual, "CharacterVirtualが無効となっており、ワールド座標取得に失敗しました。", {});

	return Utility::JoltRVec3ToDirectXMathVector3(l_characterVirtual->GetPosition());
}
FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsCharacterVirtualRegistry::FetchVALCharacterVirtualLinearVelocity(const TypeAlias::StorageID a_characterVirtualStorageID) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_characterVirtualStorageID         == Constant::k_invalidStorageID,                     "StorageIDが無効なため、速度取得に失敗しました。",      {});
	FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtualRecordList.size() <= static_cast<size_t>(a_characterVirtualStorageID), "リストのサイズを超えており、速度取得に失敗しました。", {});

	const auto& l_characterVirtual = m_characterVirtualRecordList[a_characterVirtualStorageID].m_characterVirtual;

	FWK_ASSERT_RETURN_VALUE_IF(!l_characterVirtual, "CharacterVirtualが無効となっており、速度取得に失敗しました。", {});

	return Utility::JoltVec3ToDirectXMathVector3(l_characterVirtual->GetLinearVelocity());
}
bool FWK::Physics::PhysicsCharacterVirtualRegistry::FetchVALIsCharacterVirtualOnGround(const TypeAlias::StorageID a_characterVirtualStorageID) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_characterVirtualStorageID         == Constant::k_invalidStorageID,                     "StorageIDが無効なため、設置状態取得に失敗しました。",      {});
	FWK_ASSERT_RETURN_VALUE_IF(m_characterVirtualRecordList.size() <= static_cast<size_t>(a_characterVirtualStorageID), "リストのサイズを超えており、設置状態取得に失敗しました。", {});

	const auto& l_characterVirtual = m_characterVirtualRecordList[a_characterVirtualStorageID].m_characterVirtual;

	FWK_ASSERT_RETURN_VALUE_IF(!l_characterVirtual, "CharacterVirtualが無効となっており、設置状態取得に失敗しました。", {});

	return l_characterVirtual->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
}

bool FWK::Physics::PhysicsCharacterVirtualRegistry::SetupStorage()
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_storageIDAllocator.Create(), "CharacterVirtual用StorageIDAllocatorの作成に失敗しました。", false);

	m_characterVirtualRecordList.clear();

	// StorageIDをそのまま配列Indexとして使用するため、
	// reserveではなくresizeを使用する。
	m_characterVirtualRecordList.resize(static_cast<std::size_t>(m_storageIDAllocator.GetVALStorageIDCapacity()));

	return true;
}

void FWK::Physics::PhysicsCharacterVirtualRegistry::UpdateCharacterVirtualRecord(const Struct::PhysicsCharacterVirtualUpdateData& a_updateData, const float a_deltaTime, CharacterVirtualRecord& a_characterVirtualRecord)
{
	auto& l_characterVirtual = a_characterVirtualRecord.m_characterVirtual;

	FWK_ASSERT_RETURN_IF(!l_characterVirtual, "CharacterVirtualがnullptrのため、更新に失敗しました。");

	const auto& l_physicsSystem       = m_physicsSystem.lock      ();
	const auto& l_physicsLayerSetting = m_physicsLayerSetting.lock();
	const auto& l_tempAllocator       = m_tempAllocator.lock      ();

	FWK_ASSERT_RETURN_IF(!l_physicsSystem,       "PhysicsSystemが無効なため、CharacterVirtualの更新に失敗しました。");
	FWK_ASSERT_RETURN_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、CharacterVirtualの更新に失敗しました。");
	FWK_ASSERT_RETURN_IF(!l_tempAllocator,       "TempAllocatorが無効なため、CharacterVirtualの更新に失敗しました。");

	const auto  l_characterObjectLayer  = l_physicsLayerSetting->FetchVALObjectLayer      (Enum::PhysicsObjectLayerType::CharacterObject);
	const auto& l_braodPhaseLayerFilter = l_physicsSystem->GetDefaultBroadPhaseLayerFilter(l_characterObjectLayer);
	const auto& l_objectLayerFilter     = l_physicsSystem->GetDefaultLayerFilter          (l_characterObjectLayer);

	const JPH::BodyFilter  l_bodyFilter  = {};
	const JPH::ShapeFilter l_shapeFilter = {};

	const auto& l_desiredVelocity = Utility::DirectXMathVector3ToJoltVec3(a_updateData.m_desiredVelocity);
}

void FWK::Physics::PhysicsCharacterVirtualRegistry::ReleaseAllCharacterVirtuals()
{
	for (std::size_t l_index = 0ULL; l_index < m_characterVirtualRecordList.size(); ++l_index)
	{
		auto& l_characterVirtualRecord = m_characterVirtualRecordList[l_index];

		if (!l_characterVirtualRecord.m_characterVirtual) { continue; }

		l_characterVirtualRecord.m_characterVirtual       = nullptr;
		l_characterVirtualRecord.m_extendedUpdateSettings = {};

		// ストレージIDAllcoatorにIDを再利用可能状態にする
		m_storageIDAllocator.Release(static_cast<TypeAlias::StorageID>(l_index));
	}

	m_characterVirtualRecordList.clear();
}