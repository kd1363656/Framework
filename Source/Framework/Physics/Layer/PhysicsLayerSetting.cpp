#include "PhysicsLayerSetting.h"

FWK::Physics::PhysicsLayerSetting::PhysicsLayerSetting() : 
	m_broadPhaseLayerInterface(static_cast<JPH::uint>(Enum::PhysicsObjectLayerType::Count), static_cast<JPH::uint>(Enum::PhysicsBroadPhaseLayerType::Count)),
	m_objectLayerPairFilter   (static_cast<JPH::uint>(Enum::PhysicsObjectLayerType::Count))
{}

FWK::Physics::PhysicsLayerSetting::~PhysicsLayerSetting() = default;

void FWK::Physics::PhysicsLayerSetting::INIT()
{
	// ObjectLayerをBroadPhaseLayerへ対応付ける
	SetupBroadPhaseLayerMapping();

	// ObjectLayer同士の衝突可否を設定する。
	SetupObjectLayerCollisionFilter();

	// ObjectLayerVSBroadPhaseLayerのFilterを作る
	// このFilterは上2つの設定を元に作るため、必ず最後に生成する
	SetupObjectVSBroadPhaseLayerFilter();	
}

JPH::ObjectLayer FWK::Physics::PhysicsLayerSetting::FetchVALObjectLayer(const Enum::PhysicsObjectLayerType a_objectLayerType) const
{
	FWK_ASSERT_RETURN_VALUE_IF(a_objectLayerType == Enum::PhysicsObjectLayerType::Invalid ||
                               a_objectLayerType == Enum::PhysicsObjectLayerType::Count,
                               "無効なPhysicsObjectLayerTypeが指定されており、ObjectLayerの取得に失敗しました。",
                               ConvertToJoltObjectLayer(Enum::PhysicsObjectLayerType::Invalid));

	return ConvertToJoltObjectLayer(a_objectLayerType);
}

void FWK::Physics::PhysicsLayerSetting::SetupBroadPhaseLayerMapping()
{
	// Invalidは実運用では使用しない。
	// ただし、Enum値とJolt側のLayer番号を
	// 単純に対応させるため、Invalid同士を登録しておく。
	m_broadPhaseLayerInterface.MapObjectToBroadPhaseLayer(ConvertToJoltObjectLayer(Enum::PhysicsObjectLayerType::Invalid), ConvertToJoltBroadPhaseLayer(Enum::PhysicsBroadPhaseLayerType::Invalid));

	// 床、壁、見えないBoxCollider、MeshColliderなどの
	// 動かないColliderをStatic BroadPhaseLayerへ登録する。
	m_broadPhaseLayerInterface.MapObjectToBroadPhaseLayer(ConvertToJoltObjectLayer(Enum::PhysicsObjectLayerType::StaticObject), ConvertToJoltBroadPhaseLayer(Enum::PhysicsBroadPhaseLayerType::Static));

	// CharacterObjectはCharacterVirtualが衝突Queryを
	// 実行するときに使用するObjectLayer。
	// CharacterVirtual自体をBodyとして登録するわけではないが、
	// BroadPhaseLayerInterfaceTableではすべてのObjectLayerに
	// 対応するBroadPhaseLayerを設定する必要があるため、
	// Static BroadPhaseLayerへ割り当てる。
	m_broadPhaseLayerInterface.MapObjectToBroadPhaseLayer(ConvertToJoltObjectLayer(Enum::PhysicsObjectLayerType::CharacterObject), ConvertToJoltBroadPhaseLayer(Enum::PhysicsBroadPhaseLayerType::Static));
}
void FWK::Physics::PhysicsLayerSetting::SetupObjectLayerCollisionFilter()
{
	// ObjectLayerPairFilterTableは、
	// 初期状態ではすべての衝突が無効になっている。
	// CharacterVirtualの衝突Queryから、
	// StaticObjectを検出できる組み合わせだけを有効化する。
	EnableObjectLayerCollision(Enum::PhysicsObjectLayerType::StaticObject, Enum::PhysicsObjectLayerType::CharacterObject);
}
void FWK::Physics::PhysicsLayerSetting::SetupObjectVSBroadPhaseLayerFilter()
{
	// ObjectVSBroadPhaseLayerFilterTableは、BroadPhaseLayerInterfaceとObjectLayerPairFilterをもとに作られる
	m_objectVsBroadPhaseLayerFilter = std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(m_broadPhaseLayerInterface,
																							    static_cast<JPH::uint>(Enum::PhysicsBroadPhaseLayerType::Count),
																								m_objectLayerPairFilter,
																								static_cast<JPH::uint>(Enum::PhysicsObjectLayerType::Count));
}

void FWK::Physics::PhysicsLayerSetting::EnableObjectLayerCollision(const Enum::PhysicsObjectLayerType a_layerA, const Enum::PhysicsObjectLayerType a_layerB)
{
	FWK_ASSERT_RETURN_IF(a_layerA == Enum::PhysicsObjectLayerType::Invalid ||
                         a_layerB == Enum::PhysicsObjectLayerType::Invalid ||
                         a_layerA == Enum::PhysicsObjectLayerType::Count   ||
                         a_layerB == Enum::PhysicsObjectLayerType::Count,
                         "無効なPhysicsObjectLayerType同士の衝突を有効化しようとしました");

	m_objectLayerPairFilter.EnableCollision(ConvertToJoltObjectLayer(a_layerA), ConvertToJoltObjectLayer(a_layerB));
}

JPH::ObjectLayer FWK::Physics::PhysicsLayerSetting::ConvertToJoltObjectLayer(const Enum::PhysicsObjectLayerType a_objectLayerType) const
{
	return static_cast<JPH::ObjectLayer>(a_objectLayerType);
}
JPH::BroadPhaseLayer FWK::Physics::PhysicsLayerSetting::ConvertToJoltBroadPhaseLayer(const Enum::PhysicsBroadPhaseLayerType a_broadPhaseLayerType) const
{
	return JPH::BroadPhaseLayer(static_cast<JPH::BroadPhaseLayer::Type>(a_broadPhaseLayerType));
}