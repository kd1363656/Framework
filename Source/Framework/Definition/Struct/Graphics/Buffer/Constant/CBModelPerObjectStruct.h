#pragma once

namespace FWK::Struct
{
    struct CBModelPerObject final
    {
        TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;

        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        TypeAlias::DescriptorIndex m_materialBufferSRVDescriptorIndex          = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        float                      m_worldMaxScale                          = Constant::k_defaultMeshletBoundWorldMaxScale;
        float                      m_worldOrientationSign                   = Constant::k_normalModelWorldOrientationSign;

        std::uint32_t m_meshletCount  = Constant::k_defaultMeshletCount;
        std::uint32_t m_meshletOffset = Constant::k_initialMeshLODMeshletOffset;
        float         m_padding       = Constant::k_initialCBModelPerObjectPadding;
        float         m_padding2      = Constant::k_initialCBModelPerObjectPadding;

        // メッシュシェーダーの中でスキニングする場合に使う値
        // m_skinningMatrixBufferSRVDescriptorIndex : 事前計算したスキニング行列のBuffer
        // m_skinningMatrixOffset                   : このMesh、このコマのスキニング行列が始まる番号
        // m_isGPUSkinning                          : 1ならスキニングする(Meshlet単位のカリングは、動いた形と合わないため行わない)
        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_skinningMatrixOffset                   = Constant::k_initialCBModelPerObjectSkinningValue;
        std::uint32_t              m_isGPUSkinning                          = Constant::k_initialCBModelPerObjectSkinningValue;
        std::uint32_t              m_skinningPadding                        = Constant::k_initialCBModelPerObjectSkinningValue;
    };

    static_assert(sizeof(CBModelPerObject) == 192ULL, "CBModelPerObjectのサイズがHLSL側と一致しません。");
}