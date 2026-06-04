#pragma once

namespace FWK::Constant
{
	// ID3D12Deviceを使って作るコマンドキューなどの各種GPUオブジェクトで
	// 共通使用する既定のGPUノード指定値
	// GPUノードマスクは「どのGPUノードを対象にするか」を表す値、
	// このプロジェクトでは単一GPU前提で作成するため固定値にしている
	inline constexpr UINT k_defaultGPUNodeMask = 0U;

	inline constexpr TypeAlias::DescriptorIndex k_invalidDescriptorIndex = std::numeric_limits<TypeAlias::DescriptorIndex>::max();
}