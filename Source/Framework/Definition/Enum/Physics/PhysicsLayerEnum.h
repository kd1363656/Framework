#pragma once

namespace FWK::Enum
{
	// Joltには最終的にJPH::ObjectLayerとして渡すが、
	// プロジェクト側では意味が分かるEnumで管理する
	enum class PhysicsObjectLayerType
	{
		Invalid,

		// 動かないオブジェクトに使用、
		// マップ、壁、床、など重力を当てる必要がなく基本的に動かないものに付与
		StaticObject,

		// CharacterVirtualのQuery用ObjectLayer
		CharacterObject,

		Count,
	};

	// BroadPhaseLayerはJoltのBroadPhase空間分割用の大分類
	enum class PhysicsBroadPhaseLayerType 
	{
		Invalid,

		// 基本的に動かない、重力がかからないものに適用
		Static,

		Count
	};
}