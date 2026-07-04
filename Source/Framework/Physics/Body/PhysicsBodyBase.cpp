#include "PhysicsBodyBase.h"

FWK::Graphics::PhysicsBodyBase::PhysicsBodyBase(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const JPH::BodyID& a_bodyID) : 
	m_bodyID(a_bodyID)
{}

FWK::Graphics::PhysicsBodyBase::PhysicsBodyBase() = default;
FWK::Graphics::PhysicsBodyBase::~PhysicsBodyBase()
{
	// 使用していたBodyIDをPhysicsSystemに返却する
	ReleaseBody();
}

FWK::TypeAlias::Math::Vector3 FWK::Graphics::PhysicsBodyBase::FetchVALWorldPosition() const
{
	FWK_ASSERT_RETURN_VALUE_IF(m_bodyID.IsInvalid(), "BodyIDが無効なため、Bodyのワールド座標取得に失敗しました。", {});

	const auto& l_graphicsManager = FWK::Physics::PhysicsManager::GetInstance();
	const auto& l_physicsSystem   = l_graphicsManager.GetREFPhysicsSystem    ();

	const auto& l_bodyInterface = l_physicsSystem.GetBodyInterface();
	const auto& l_worldPosition = l_bodyInterface.GetPosition     (m_bodyID);

	return Utility::JoltRVec3ToDirectXMathVector3(l_worldPosition);
}

bool FWK::Graphics::PhysicsBodyBase::FetchVALIsValid() const
{
	return m_bodyID.IsInvalid();
}

void FWK::Graphics::PhysicsBodyBase::ReleaseBody()
{
	// 生成に失敗した、もしくは既に解放済みのBodyでは何もしない
	if (m_bodyID.IsInvalid()) { return; }

	auto& l_graphicsManager = FWK::Physics::PhysicsManager::GetInstance   ();
	auto& l_physicsSystem   = l_graphicsManager.GetMutableREFPhysicsSystem();

	auto& l_bodyInterface = l_physicsSystem.GetBodyInterface();

	// 追加されている者かどうかを判断して追加されているものならRemove
	if (l_bodyInterface.IsAdded(m_bodyID))
	{
		l_bodyInterface.RemoveBody(m_bodyID);
	}

	// RmoveBody()だけでは、Jolt内部のBody本体は残っているため、
	// 最後にDestroyBodyを呼び、Body本体を破棄
	l_bodyInterface.DestroyBody(m_bodyID);

	// 二重開放をふせっぐため無効値へ戻す
	m_bodyID = {};
}