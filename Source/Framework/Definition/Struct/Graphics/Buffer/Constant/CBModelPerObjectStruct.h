#pragma once

namespace FWK::Struct
{
    struct CBModelPerObject final
    {
        static constexpr float k_initialPadding = 0.0F;

        static constexpr std::uint32_t k_initialSkinningValue = 0U;

        TypeAlias::Math::Matrix m_worldMatrix = TypeAlias::Math::Matrix::Identity;

        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        // m_materialBufferSRVDescriptorIndex : MaterialのGPUDataBuffer
        // Materialの値(色やテクスチャ)はMaterialの種類ごとに違うため、PixelShaderが自身のMaterialの型として読む
        TypeAlias::DescriptorIndex m_materialBufferSRVDescriptorIndex          = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        float                      m_worldMaxScale                          = Constant::k_defaultMeshletBoundWorldMaxScale;
        float                      m_worldOrientationSign                   = Constant::k_normalModelWorldOrientationSign;

        // m_meshletOffset : 描画に使うLODのMeshletが、MeshletBufferの何番目から始まるか
        // m_meshletCount  : そのLODのMeshlet数
        std::uint32_t m_meshletCount  = Constant::k_defaultMeshletCount;
        std::uint32_t m_meshletOffset = Struct::ModelMeshLOD::k_initialMeshletOffset;
        float         m_padding       = k_initialPadding;
        float         m_padding2      = k_initialPadding;

        // メッシュシェーダーの中でスキニングする場合に使う値
        // m_skinningMatrixBufferSRVDescriptorIndex : 事前計算したスキニング行列のBuffer
        // m_skinningMatrixOffset                   : このMesh、このコマのスキニング行列が始まる番号
        // m_isGPUSkinning                          : 1ならスキニングする(Meshlet単位のカリングは、動いた形と合わないため行わない)
        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_skinningMatrixOffset                   = k_initialSkinningValue;
        std::uint32_t              m_isGPUSkinning                          = k_initialSkinningValue;
        std::uint32_t              m_skinningPadding                        = k_initialSkinningValue;
    };

    static_assert(sizeof(CBModelPerObject) == 192ULL, "CBModelPerObjectのサイズがHLSL側と一致しません。");
}