#pragma once

namespace FWK::Struct
{
	struct BackBuffer final
	{
		TypeAlias::ComPtr<ID3D12Resource2> m_backBufferResource = nullptr;

		D3D12_RESOURCE_STATES m_currentResourceState = D3D12_RESOURCE_STATE_PRESENT;

		TypeAlias::DescriptorIndex m_rtvDescriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}