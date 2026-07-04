#include "PhysicsBodyRegistry.h"

FWK::Physics::PhysicsBodyRegistry::PhysicsBodyRegistry() : 
	m_activeBodyIDIndexMap(),

	m_activeBodyIDList(),

	m_physicsSystem      (),
	m_physicsLayerSetting(),

	m_bodyCreator(),

	m_isSetup(false)
{}
FWK::Physics::PhysicsBodyRegistry::~PhysicsBodyRegistry()
{
	// 既に解放されている場合はreturn
	if (!m_isSetup) { return; }

	ReleaseAllBodies();

	m_isSetup = false;
}

bool FWK::Physics::PhysicsBodyRegistry::Setup(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const std::shared_ptr<PhysicsLayerSetting>& a_physicsLayerSetting)
{
	FWK_ASSERT_RETURN_VALUE_IF(m_isSetup,              "セットアップ処理をもう一度行おうとしており、Setup処理に失敗しました。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_physicsSystem,       "PhysicsSystemが無効となっておりSetup処理に失敗しました。",              false);
	FWK_ASSERT_RETURN_VALUE_IF(!a_physicsLayerSetting, "PhysicsLayerSettingが無効となっておりSetup処理に失敗しました。",        false);

	m_physicsSystem       = a_physicsSystem;
	m_physicsLayerSetting = a_physicsLayerSetting;

	m_isSetup = true;

	return true;
}

void FWK::Physics::PhysicsBodyRegistry::ReleaseBody(Struct::PhysicsBodyHandle& a_bodyHandle)
{
	FWK_ASSERT_RETURN_IF(!m_isSetup,						"PhysicsBodyRegistryが設定されていないため、Body解放に失敗しました。");
	FWK_ASSERT_RETURN_IF(!a_bodyHandle.m_isValid,           "PhysicsBodyHandleが無効なため、Body解放に失敗しました。");
	FWK_ASSERT_RETURN_IF(a_bodyHandle.m_bodyID.IsInvalid(), "BodyIDが無効なため、Body解放に失敗しました。");

	const auto& l_physicsSystem = m_physicsSystem.lock();

	FWK_ASSERT_RETURN_IF(!l_physicsSystem, "PhysicsSystemが無効なため、Body解放に失敗しました。");

	auto& l_bodyInterface = l_physicsSystem->GetBodyInterface();

	const auto l_bodyID = a_bodyHandle.m_bodyID;

	// BodyがPhysicsSystemに追加されている場合は、まずPhysicsSystemから外す。
	// RemoveBodyを呼ぶと、以後このBodyは物理更新・衝突判定の対象外になる。
	if (l_bodyInterface.IsAdded(l_bodyID))
	{
		l_bodyInterface.RemoveBody(l_bodyID);
	}

	// Body本体を破棄する
	// RemoveBodyだけでは、Jolt内部にBodyが残るためDestroyBodyも必要
	l_bodyInterface.DestroyBody(l_bodyID);

	UnregisterActiveBodyID(l_bodyID);

	// 呼び出し側が削除済みBodyIDを持ち続けないようにする。
	a_bodyHandle.m_bodyID  = JPH::BodyID();
	a_bodyHandle.m_isValid = false;
}

FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyRegistry::CreateStaticSphereBody(const TypeAlias::Math::Vector3& a_worldPosition, const float a_radius)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isSetup, "PhysicsBodyRegistryが設定されていないため、StaticSphereBodyの作成に失敗しました。", {});

	const auto& l_physicsSystem		  = m_physicsSystem.lock	  ();
	const auto& l_physicsLayerSetting = m_physicsLayerSetting.lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsSystem,       "PhysicsSystemが無効なため、StaticSphereBodyの作成に失敗しました。",		     {});
	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingがnullptrのため、StaticSphereBodyの作成に失敗しました。", {});

	const auto l_bodyHandle = m_bodyCreator.CreateStaticSphereBody(*l_physicsLayerSetting,
																   a_worldPosition,
																   a_radius,
																   *l_physicsSystem);

	RegisterActiveBodyID(l_bodyHandle);

	return l_bodyHandle;
}
FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyRegistry::CreateStaticBoxBody(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Vector3& a_halfExtent)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isSetup, "PhysicsBodyRegistryが設定されていないため、StaticBoxBodyの作成に失敗しました。", {});

	const auto& l_physicsSystem		  = m_physicsSystem.lock	  ();
	const auto& l_physicsLayerSetting = m_physicsLayerSetting.lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsSystem,       "PhysicsSystemが無効なため、StaticBoxBodyの作成に失敗しました。",          {});
	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingがnullptrのため、StaticBoxBodyの作成に失敗しました。", {});

	const auto l_bodyHandle = m_bodyCreator.CreateStaticBoxBody(*l_physicsLayerSetting,
															    a_worldPosition,
															    a_halfExtent,
															    *l_physicsSystem);

	RegisterActiveBodyID(l_bodyHandle);

	return l_bodyHandle;
}
FWK::Struct::PhysicsBodyHandle FWK::Physics::PhysicsBodyRegistry::CreateStaticCapsuleBody(const TypeAlias::Math::Vector3& a_worldPosition, const float a_halfHeightOfCylinder, const float a_radius)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isSetup, "PhysicsBodyRegistryが設定されていないため、StaticCapsuleBodyの作成に失敗しました。", {});

	const auto& l_physicsSystem		  = m_physicsSystem.lock	  ();
	const auto& l_physicsLayerSetting = m_physicsLayerSetting.lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsSystem,       "PhysicsSystemが無効なため、StaticCapsuleBodyの作成に失敗しました。",          {});
	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingがnullptrのため、StaticCapsuleBodyの作成に失敗しました。", {});

	const auto l_bodyHandle = m_bodyCreator.CreateStaticCapsuleBody(*l_physicsLayerSetting,
												                    a_worldPosition,
												                    a_halfHeightOfCylinder,
												                    a_radius,
												                    *l_physicsSystem);

	RegisterActiveBodyID(l_bodyHandle);

	return l_bodyHandle;
}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsBodyRegistry::FetchVALBodyWorldPosition(const Struct::PhysicsBodyHandle& a_bodyHandle) const
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_isSetup,						  "PhysicsBodyRegistryが設定されていないため、Bodyの座標取得に失敗しました。", {});
	FWK_ASSERT_RETURN_VALUE_IF(!a_bodyHandle.m_isValid,           "PhysicsBodyHandleが無効なため、Bodyの座標取得に失敗しました。",             {});
	FWK_ASSERT_RETURN_VALUE_IF(a_bodyHandle.m_bodyID.IsInvalid(), "BodyIDが無効なため、Bodyの座標取得に失敗しました。",			               {});

	const auto& l_physicsSystem = m_physicsSystem.lock();

	// JoltのBodyInterfaceを取得する
	// BodyInterfaceは、BodyIDを使ってBodyの位置・回転。速度などを操作する入口。
	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsSystem, "PhysicsSystemが無効なため、StaticCapsuleBodyの作成に失敗しました。", {});

	const auto& l_bodyInterface = l_physicsSystem->GetBodyInterface();

	// Jolt側の現在座標を取得する
	// GetPosition()はBodyのワールド座標を返す
	const auto& l_bodyPosition = l_bodyInterface.GetPosition(a_bodyHandle.m_bodyID);

	return Utility::JoltRVec3ToDirectXMathVector3(l_bodyPosition);
}

void FWK::Physics::PhysicsBodyRegistry::RegisterActiveBodyID(const Struct::PhysicsBodyHandle& a_bodyHandle)
{
	if (!a_bodyHandle.m_isValid ||
		a_bodyHandle.m_bodyID.IsInvalid()) 
	{
		return;
	}

	const auto l_bodyIDKey = FetchVALBodyIDKey(a_bodyHandle.m_bodyID);

	// すでに登録済みなら何もしない。
	// unordered_mapなので、登録済み確認が高速
	if (m_activeBodyIDIndexMap.contains(l_bodyIDKey)) { return; }

	const auto l_bodyIDIndex = m_activeBodyIDList.size();

	m_activeBodyIDList.emplace_back   (a_bodyHandle.m_bodyID);
	m_activeBodyIDIndexMap.try_emplace(l_bodyIDKey, l_bodyIDIndex);
}

void FWK::Physics::PhysicsBodyRegistry::UnregisterActiveBodyID(const JPH::BodyID & a_bodyID)
{
	if (a_bodyID.IsInvalid()) { return; }

	const auto  l_removeBodyIDKey = FetchVALBodyIDKey		   (a_bodyID);
	const auto& l_itr             = m_activeBodyIDIndexMap.find(l_removeBodyIDKey);

	// マップ内になければreturn
	if (l_itr == m_activeBodyIDIndexMap.end()) { return; }

	// リストから削除したいインデックスをマップから取得
	const auto l_removeIndex = l_itr->second;
	
	if (const auto l_lastIndex = m_activeBodyIDList.size() - k_lastElementIndexOffset;
		l_removeIndex != l_lastIndex)
	{
		// 最後尾のインデックスが持つBodyIDを取得
		const auto l_lastBodyID    = m_activeBodyIDList[l_lastIndex];
		const auto l_lastBodyIDKey = FetchVALBodyIDKey(l_lastBodyID);

		// 順番に意味がないので、削除したいBodyIDと最後尾のBodyIDを入れ替える
		std::swap(m_activeBodyIDList[l_removeIndex], m_activeBodyIDList[l_lastIndex]);
		
		// リムーブするIndexと最後尾を入れ替えたので
		// 最後尾から削除位置へ移動してきたBodyIDのIndexを更新する
		m_activeBodyIDIndexMap[l_lastBodyIDKey] = l_removeIndex;
	}

	// Swap後、削除対象BodyIDは最後尾にいる。
	// そのためpop_backだけで高速に削除できる
	m_activeBodyIDList.pop_back ();
	m_activeBodyIDIndexMap.erase(l_itr);
}

void FWK::Physics::PhysicsBodyRegistry::ReleaseAllBodies()
{
	const auto& l_physicsSystem = m_physicsSystem.lock();

	FWK_ASSERT_RETURN_IF(!l_physicsSystem, "PhysicsSystemが無効なため、ReleaseAllBody処理に失敗しました。");

	auto& l_bodyInterface = l_physicsSystem->GetBodyInterface();

	for (const auto& l_bodyID : m_activeBodyIDList)
	{
		if (l_bodyID.IsInvalid()) { continue; }

		if (l_bodyInterface.IsAdded(l_bodyID))
		{
			l_bodyInterface.RemoveBody(l_bodyID);
		}

		l_bodyInterface.DestroyBody(l_bodyID);
	}

	m_activeBodyIDList.clear    ();
	m_activeBodyIDIndexMap.clear();
}

std::uint32_t FWK::Physics::PhysicsBodyRegistry::FetchVALBodyIDKey(const JPH::BodyID & a_bodyID) const
{
	// BodyIDをunordered_map用のキーに変換する
	// BodyIDのIndexだけを使うと、Destroy後に同じIndexが再利用されたときに危ない。
	// GetIndexAndSequenceNumber()はIndex + SequenceNumberを含む値を返す
	return a_bodyID.GetIndexAndSequenceNumber();
}