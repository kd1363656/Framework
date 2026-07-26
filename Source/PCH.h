#pragma once

//================================================
// Windows
//================================================
#define NOMINMAX
#include <Windows.h>
#include <wrl/client.h>
#include <cassert>

//================================================
// STL
//================================================
#include <cstring>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <array>
#include <deque>
#include <memory>
#include <limits>
#include <filesystem>
#include <fstream>
#include <thread>
#include <queue>
#include <unordered_set>
#include <format>
#include <source_location>

//================================================
// 外部ライブラリ
//================================================
// json
#include <json.hpp>

//================================================
// DirectX12
//================================================
#include <d3d12.h>
#include <dxgi1_6.h>

//================================================
// D3DX12
//================================================
#include <d3dx12.h>

//================================================
// DXC
//================================================
#include <dxcapi.h>

//================================================
// DirectXTex
//================================================
#include <DirectXTex.h>

//================================================
// D3D12MemoryAllocator
//================================================
#include <D3D12MemAlloc.h>

//================================================
// DirectXTK12
//================================================
#include <SimpleMath.h>

//================================================
// UFBX
//================================================
#include <ufbx.h>

//================================================
// meshoptimizer
//================================================
#include <meshoptimizer.h>

//================================================
// JoltPhysics
//================================================
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>

// Body関係
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>

// Shape関係
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

// CharacterVirtual関係
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>

// JoltMath関係
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Quat.h>

// JoltDebugDraw関係
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Math/Mat44.h>
#include <Jolt/Physics/Body/BodyManager.h>

//================================================
// IMGUI
//================================================
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_PLACEMENT_NEW
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <imgui_stdlib.h>

//================================================
// フレームワーク
//================================================
#include "Framework/Framework.h"

//================================================
// リンカー設定
//================================================
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")