#pragma once

namespace FWK::Graphics
{
    class ModelMeshGPUData final
    {
    public:

         ModelMeshGPUData() = default;
        ~ModelMeshGPUData() = default;

        void SetVertexBufferSRVDescriptorIndex           (const TypeAlias::DescriptorIndex a_set) { m_vertexBufferSRVDescriptorIndex            = a_set; }
        void SetMeshletBufferSRVDescriptorIndex          (const TypeAlias::DescriptorIndex a_set) { m_meshletBufferSRVDescriptorIndex           = a_set; }
        void SetUniqueVertexIndexBufferSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_uniqueVertexIndexBufferSRVDescriptorIndex = a_set; }
        void SetPrimitiveIndexBufferSRVDescriptorIndex   (const TypeAlias::DescriptorIndex a_set) { m_primitiveIndexBufferSRVDescriptorIndex    = a_set; }
        void SetMeshletBoundsBufferSRVDescriptorIndex    (const TypeAlias::DescriptorIndex a_set) { m_meshletBoundsBufferSRVDescriptorIndex     = a_set; }

        void SetMeshletCount(const std::uint32_t a_set) { m_meshletCount = a_set; }

    private:

        static constexpr std::uint32_t k_initialMeshletCount = 0U;

        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex     = DescriptorHeap::k_invalidDescriptorIndex;

        std::uint32_t m_meshletCount = k_initialMeshletCount;

        FWK_DEFINE_MODEL_RENDER_TABLE_INFO(ModelMeshGPUData)
    };
}