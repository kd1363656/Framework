#include "PhysicsBodyBase.h"

FWK::Physics::PhysicsBodyBase::PhysicsBodyBase() :
	m_bodyID(),

	m_createWorldPosition()
{}
FWK::Physics::PhysicsBodyBase::~PhysicsBodyBase()
{
	// 使用していたBodyIDをPhysicsSystemに返却する
	ReleaseBody();
}

FWK::TypeAlias::Math::Vector3 FWK::Physics::PhysicsBodyBase::FetchVALWorldPosition() const
{
	FWK_ASSERT_RETURN_VALUE_IF(m_bodyID.IsInvalid(), "BodyIDが無効なため、Bodyのワールド座標取得に失敗しました。", {});

	const auto& l_graphicsManager = FWK::Physics::PhysicsManager::GetInstance();
	const auto& l_physicsSystem   = l_graphicsManager.GetREFPhysicsSystem    ();

	const auto& l_bodyInterface = l_physicsSystem.GetBodyInterface();
	const auto& l_worldPosition = l_bodyInterface.GetPosition     (m_bodyID);

	return Utility::JoltRVec3ToDirectXMathVector3(l_worldPosition);
}

bool FWK::Physics::PhysicsBodyBase::ApplyBodyShape(const JPH::RefConst<JPH::Shape>& a_shape, const JPH::EActivation a_activationMode, const bool a_isUpdateMassProperties)
{
	FWK_ASSERT_RETURN_VALUE_IF(!a_shape,             "変更後のShapeが無効なため、Shapeの変更に失敗しました。",  false);
	FWK_ASSERT_RETURN_VALUE_IF(m_bodyID.IsInvalid(), "Bodyが作成されていないため、Shapeの変更に失敗しました。", false);

	auto& l_physicsManager = PhysicsManager::GetInstance                ();
	auto& l_physicsSystem  = l_physicsManager.GetMutableREFPhysicsSystem();
	auto& l_bodyInterface  = l_physicsSystem.GetBodyInterface		    ();

	// 同じBodyIDを維持したまま、新しいShapeへ交換する
	l_bodyInterface.SetShape(GetREFBodyID(),
							 a_shape.GetPtr(),
							 a_isUpdateMassProperties,
							 a_activationMode);

	return true;
}

void FWK::Physics::PhysicsBodyBase::ReleaseBody()
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