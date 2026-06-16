#pragma once

namespace FWK::Struct
{
	struct StructuredBufferResource final
	{
		// StructuredBufferとして扱うGPUリソースとD3D12MA側Allocation
		GPUResource m_bufferGPUResource = {};

		// StructuredBuffer用SRVのDescriptorIndex
		TypeAlias::DescriptorIndex m_srvDescriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}