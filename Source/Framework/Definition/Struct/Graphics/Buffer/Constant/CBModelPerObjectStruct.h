#pragma once

namespace FWK::Struct
{
    struct CBModelPerObject final
    {
        TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;

        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        TypeAlias::Math::Color m_baseColorFactor = Constant::k_whiteColor;

        float                      m_roughnessFactor                    = Constant::k_defaultModelMaterialRoughnessFactor;
        float                      m_metallicFactor                     = Constant::k_defaultModelMaterialMetallicFactor;
        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex     = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex    = Constant::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex     = Constant::k_invalidDescriptorIndex;
        float                      m_worldMaxScale                             = Constant::k_defaultMeshletBoundWorldMaxScale;

        float                    m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;
        std::uint32_t            m_meshletCount         = Constant::k_defaultMeshletCount;
        TypeAlias::Math::Vector2 m_padding              = TypeAlias::Math::Vector2::Zero;
    };
}