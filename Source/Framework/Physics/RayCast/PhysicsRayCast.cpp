#include "PhysicsRayCast.h"

std::vector<FWK::Struct::PhysicsRayCastHitResult> FWK::Physics::PhysicsRayCast::CastAllHitBodies(const Struct::PhysicsRay& a_physicsRay) const
{
	std::vector<Struct::PhysicsRayCastHitResult> l_hitResultList = {};

	JPH::RRayCast l_rRayCast = {};

	if (!CreateJoltRay(a_physicsRay, l_rRayCast)) { return l_hitResultList; }

	const auto& l_physicsManager      = PhysicsManager::GetInstance               ();
	const auto& l_physicsLayerSetting = l_physicsManager.GetREFPhysicsLayerSetting();
	const auto& l_physicsSystem       = l_physicsManager.GetREFPhysicsSystem      ();

	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、全BodyのRayCastに失敗しました。", l_hitResultList);

	const auto  l_rayQueryObjectLayer   = l_physicsLayerSetting->FetchVALObjectLayer     (Enum::PhysicsObjectLayerType::RayQueryObject);
	const auto& l_broadPhaseLayerFilter = l_physicsSystem.GetDefaultBroadPhaseLayerFilter(l_rayQueryObjectLayer);
	const auto& l_objectLayerFilter     = l_physicsSystem.GetDefaultLayerFilter          (l_rayQueryObjectLayer);
	const auto& l_narrowPhaseQuery      = l_physicsSystem.GetNarrowPhaseQuery            ();

	// ClosestHitとAllHitで同じRayCast設定を使用する
	// 現在はJoltの標準設定をそのまま使用する
	const auto& l_rayCastSettings = JPH::RayCastSettings{};

	// 同じBodyの複数SubShapeへHitした場合でも
	// そのBodyに対する最も手前のHitだけを保持するCollector
	JPH::ClosestHitPerBodyCollisionCollector<JPH::CastRayCollector> l_hitCollector = {};

	l_narrowPhaseQuery.CastRay(l_rRayCast,
		                       l_rayCastSettings,
		                       l_hitCollector,
		                       l_broadPhaseLayerFilter,
		                       l_objectLayerFilter);

	if (!l_hitCollector.HadHit()) { return l_hitResultList; }

	// Collectorが保持するHitをRayの視点から近い順に並べる
	l_hitCollector.Sort();

	l_hitResultList.reserve(l_hitCollector.mHits.size());

	const float l_rayLength = l_rRayCast.mDirection.Length();

	for (const auto& l_rayCastResult : l_hitCollector.mHits) 
	{
		auto l_hitResult = CreateHitResult(l_rRayCast, l_rayCastResult, l_rayLength);

		// Query後にBodyが破棄されｍ
		// BodyLockに失敗した場合はResultへ追加しない
		if (!l_hitResult.m_isHit) { continue; }

		l_hitResultList.emplace_back(std::move(l_hitResult));
	}

	return l_hitResultList;
}

FWK::Struct::PhysicsRayCastHitResult FWK::Physics::PhysicsRayCast::CastClosestHit(const Struct::PhysicsRay& a_physicsRay) const
{
	Struct::PhysicsRayCastHitResult l_hitResult = {};
	JPH::RRayCast                   l_rRayCast  = {};

	if (!CreateJoltRay(a_physicsRay, l_rRayCast)) { return l_hitResult; }

	const auto& l_physicsManager      = PhysicsManager::GetInstance               ();
	const auto& l_physicsLayerSetting = l_physicsManager.GetREFPhysicsLayerSetting();
	const auto& l_physicsSystem       = l_physicsManager.GetREFPhysicsSystem      ();

	FWK_ASSERT_RETURN_VALUE_IF(!l_physicsLayerSetting, "PhysicsLayerSettingが無効なため、最短距離のRayCastに失敗しました。", l_hitResult);

	const auto  l_rayQueryObjectLayer   = l_physicsLayerSetting->FetchVALObjectLayer     (Enum::PhysicsObjectLayerType::RayQueryObject);
	const auto& l_broadPhaseLayerFilter = l_physicsSystem.GetDefaultBroadPhaseLayerFilter(l_rayQueryObjectLayer);
	const auto& l_objectLayerFilter     = l_physicsSystem.GetDefaultLayerFilter          (l_rayQueryObjectLayer);
	const auto& l_narrowPhaseQuery      = l_physicsSystem.GetNarrowPhaseQuery            ();

	// ClosestHitとAllHitで同じRayCast設定を使用する
	// 現在はJoltの標準設定をそのまま使用する
	const auto& l_rayCastSettings = JPH::RayCastSettings{};

	// 最も手前のHitだけを保持するCollector
	JPH::ClosestHitCollisionCollector<JPH::CastRayCollector> l_hitCollector = {};

	l_narrowPhaseQuery.CastRay(l_rRayCast,
		                       l_rayCastSettings,
		                       l_hitCollector,
		                       l_broadPhaseLayerFilter,
		                       l_objectLayerFilter);

	if (!l_hitCollector.HadHit()) { return l_hitResult; }

	const float l_rayLength = l_rRayCast.mDirection.Length();

	return CreateHitResult(l_rRayCast, l_hitCollector.mHit, l_rayLength);
}

bool FWK::Physics::PhysicsRayCast::CreateJoltRay(const Struct::PhysicsRay& a_physicsRay, JPH::RRayCast& a_rRayCast) const
{
	const auto& l_startPosition      = a_physicsRay.m_startPosition;
	const auto& l_directionAndLength = a_physicsRay.m_directionAndLength;

	const auto& l_joltStartPosition = Utility::DirectXMathVector3ToJoltRVec3(l_startPosition);
	const auto& l_joltDirection     = Utility::DirectXMathVector3ToJoltRVec3(l_directionAndLength);

	FWK_ASSERT_RETURN_VALUE_IF(l_joltDirection.LengthSq() <= std::numeric_limits<float>::epsilon(), "Rayの方向と長さを表すベクトルが短すぎるため、Rayの作成に失敗しました。", false);

	// JoltのmDirectionは単位方向ではなく
	// Rayを飛ばす方向と最大距離を同時に表すベクトル
	a_rRayCast = JPH::RRayCast{ l_joltStartPosition, l_joltDirection };

	return true;
}

FWK::Struct::PhysicsRayCastHitResult FWK::Physics::PhysicsRayCast::CreateHitResult(const JPH::RRayCast& a_rRayCast, const JPH::RayCastResult& a_rayCastResult, const float a_rayLength) const
{
	Struct::PhysicsRayCastHitResult l_hitResult = {};

	const auto& l_phisicsSystem     = PhysicsManager::GetInstance         ().GetREFPhysicsSystem();
	const auto& l_bodyLockInterface = l_phisicsSystem.GetBodyLockInterface();

	// RayCast結果取得後にBodyが別Threadで破棄される可能性を考慮し、
	// BodyIDからBodyへアクセスする間はReadLockを取得する
	const auto& l_bodyLock = JPH::BodyLockRead{ l_bodyLockInterface, a_rayCastResult.mBodyID };

	if (!l_bodyLock.Succeeded()) { return l_hitResult; }

	const auto& l_body = l_bodyLock.GetBody();

	// Hit位置 = Ray開始位置 + Ray方向と長さ * Hit割合
	const auto& l_hitPosition = a_rRayCast.GetPointOnRay(a_rayCastResult.mFraction);
	
	// HitしたSubShapeとHit位置を使用して
	// Body表面のワールド空間法線を取得する
	const auto& l_surfaceNormal = l_body.GetWorldSpaceSurfaceNormal(a_rayCastResult.mSubShapeID2, l_hitPosition);

	l_hitResult.m_bodyID           = a_rayCastResult.mBodyID;
	l_hitResult.m_subShadpeID      = a_rayCastResult.mSubShapeID2;
	l_hitResult.m_hitWorldPosition = Utility::JoltRVec3ToDirectXMathVector3(l_hitPosition);
	l_hitResult.m_hitFraction      = a_rayCastResult.mFraction;
	l_hitResult.m_hitDistance      = a_rayLength * a_rayCastResult.mFraction;
	l_hitResult.m_isHit            = true;

	return l_hitResult;
}