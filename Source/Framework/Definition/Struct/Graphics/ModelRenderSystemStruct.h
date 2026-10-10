#pragma once

namespace FWK::Struct
{
    struct ModelObjectGPUData final
    {
        static constexpr float k_initialWorldMAXScale = 1.0F;

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        float m_worldMAXScale        = k_initialWorldMAXScale;
        float m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;
    };

    struct ModelMeshGPUData final
    {
        static constexpr std::uint32_t k_initialMeshletCount = 0U;

        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex     = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        std::uint32_t m_meshletCount = k_initialMeshletCount;
    };

    struct ModelStandardLitMaterialGPUData final
    {
        static constexpr float k_defaultMetallic  = 0.0F;
        static constexpr float k_defaultRoughness = 1.0F;

        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        float m_metallic  = k_defaultMetallic;
        float m_roughness = k_defaultRoughness;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct ModelStandardUnLitMaterialGPUData final
    {
        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    struct ModelDrawItem final
    {
        Struct::RCModelDrawItem m_rootConstant = {};

        D3D12_DISPATCH_MESH_ARGUMENTS m_dispatchMeshArguments = {};
    };

    struct ModelRenderTableSetting final
    {
        static constexpr UINT k_defaultCapacity = 1024U;

        Enum::ModelRenderTableType m_type = Enum::ModelRenderTableType::Invalid;

        UINT m_capacity = k_defaultCapacity;
    };
}