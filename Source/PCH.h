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
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")