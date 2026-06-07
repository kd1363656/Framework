#pragma once

namespace FWK::Struct
{
	struct DeferredResourceReleaseRecordBase
	{
		UINT64 m_retiredFenceValue = Constant::k_unusedFenceValue;
	};

	struct DescriptorIndexReleaseRecord final : public DeferredResourceReleaseRecordBase
	{
		TypeAlias::DescriptorIndex m_descriptorIndex = Constant::k_invalidDescriptorIndex;
	};
}