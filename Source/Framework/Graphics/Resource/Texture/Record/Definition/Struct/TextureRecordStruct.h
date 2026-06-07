#pragma once

namespace FWK::Struct
{
	struct GPUResource final
	{
		// D3D12MAで作成したGPUリソース
		TypeAlias::ComPtr<ID3D12Resource2> m_resource = nullptr;

		// m_resourceに対応するD3D12MA側のAllocation
		TypeAlias::ComPtr<D3D12MA::Allocation> m_allocation = nullptr;
	};	
}