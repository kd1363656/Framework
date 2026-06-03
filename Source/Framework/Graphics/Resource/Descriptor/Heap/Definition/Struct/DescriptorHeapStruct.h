#pragma once

namespace FWK::Struct
{
	struct DescriptorIndexReleaseRecord final
	{
		UINT64 m_retiredFenceValue = 0ULL;

		TypeAlias::DescriptorIndex m_descriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}