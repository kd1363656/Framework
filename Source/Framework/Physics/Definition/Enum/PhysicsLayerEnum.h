#pragma once

namespace FWK::Enum
{
	// Joltには最終的にJPH::ObjectLayerとして渡すが、
	// プロジェクト側では意味が分かるEnumで管理する
	enum class PhysicsObjectLayerType
	{
		Invalid,

		StaticObject,
		Count,
	};

	// BroadPhaseLayerはJoltのBroadPhase空間分割用の大分類
	enum class PhysicsBroadPhaseLayerType 
	{
		Invalid,

		Static,
		Count
	};
}