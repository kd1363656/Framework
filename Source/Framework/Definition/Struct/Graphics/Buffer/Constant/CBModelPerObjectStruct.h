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

        // m_lodCoveredLayerDepthOffset                    : 他のMeshに覆われている頂点を、覆われている層1つ分だけカメラから見て奥へ押し出す距離(ワールド座標。0以上)
        //                                                  頂点を「視線の向き × この値 × 覆われている層の数」だけ動かす
        // m_lodCoveredLayerCountBufferSRVDescriptorIndex : 頂点ごとの覆われている層の数のBuffer
        std::uint32_t              m_meshletCount                                 = Constant::k_defaultMeshletCount;
        std::uint32_t              m_meshletOffset                                = Constant::k_initialMeshLODMeshletOffset;
        float                      m_lodCoveredLayerDepthOffset                   = Constant::k_initialCBModelPerObjectLODCoveredLayerDepthOffset;
        TypeAlias::DescriptorIndex m_lodCoveredLayerCountBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // メッシュシェーダーの中でスキニングする場合に使う値
        // m_skinningMatrixBufferSRVDescriptorIndex : 事前計算したスキニング行列のBuffer
        // m_skinningMatrixOffset                   : このMesh、このコマのスキニング行列が始まる番号
        // m_isGPUSkinning                          : 1ならスキニングする(Meshlet単位のカリングは、動いた形と合わないため行わない)
        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_skinningMatrixOffset                   = Constant::k_initialCBModelPerObjectSkinningValue;
        std::uint32_t              m_isGPUSkinning                          = Constant::k_initialCBModelPerObjectSkinningValue;
        std::uint32_t              m_skinningPadding                        = Constant::k_initialCBModelPerObjectSkinningValue;

        // メッシュシェーダーの中でモーフする場合に使う値(モーフを持たないMeshではm_morphDeltaBufferSRVDescriptorIndexが無効)
        // m_morphWeightBufferSRVDescriptorIndex : ウェイトのBuffer(ベイクした物か、インスタンスごとに上書きする物)
        // m_morphWeightOffset                   : このインスタンスのウェイトが始まる番号(ウェイトの番号 = m_morphWeightOffset + チャンネル番号)
        TypeAlias::DescriptorIndex m_morphVertexRangeBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_morphDeltaBufferSRVDescriptorIndex       = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_morphWeightBufferSRVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_morphWeightOffset                        = Constant::k_initialCBModelPerObjectSkinningValue;
    };

    static_assert(sizeof(CBModelPerObject) == 208ULL, "CBModelPerObjectのサイズがHLSL側と一致しません。");
}