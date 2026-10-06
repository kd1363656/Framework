#pragma once

namespace FWK::Struct
{
    struct DeferredResourceReleaseRecordBase
    {
        UINT64 m_retiredFenceValue = Graphics::Fence::k_unusedFenceValue;
    };

    struct DescriptorIndexReleaseRecord final : public DeferredResourceReleaseRecordBase
    {
        TypeAlias::DescriptorIndex m_descriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct GPUResourceReleaseRecord final : public DeferredResourceReleaseRecordBase
    {
        Struct::GPUResource m_gpuResource = {};
    };
}