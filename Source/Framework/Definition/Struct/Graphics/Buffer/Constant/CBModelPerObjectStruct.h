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

        // m_lodCoveredLayerDepthOffsetRatio               : 他のMeshに覆われている頂点を、覆われている層1つ分だけカメラから見て奥へ押し出す距離の、カメラからの距離に対する割合(0以上)
        //                                                  頂点を「視線の向き × この値 × 頂点のカメラからの距離 × 覆われている層の数」だけ動かす
        // m_lodCoveredLayerCountBufferSRVDescriptorIndex : 頂点ごとの覆われている層の数のBuffer
        std::uint32_t              m_meshletCount                                 = Constant::k_defaultMeshletCount;
        std::uint32_t              m_meshletOffset                                = Constant::k_initialMeshLODMeshletOffset;
        float                      m_lodCoveredLayerDepthOffsetRatio              = Constant::k_initialCBModelPerObjectLODCoveredLayerDepthOffsetRatio;
        TypeAlias::DescriptorIndex m_lodCoveredLayerCountBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // メッシュシェーダーの中でスキニングする場合に使う値
        // m_skinningMatrixBufferSRVDescriptorIndex : 事前計算したスキニング行列のBuffer
        // m_skinningMatrixOffset                   : このMesh、このコマのスキニング行列が始まる番号
        // m_isGPUSkinning                          : 1ならスキニングする(Meshlet単位のカリングは、動いた形と合わないため行わない)
        // m_cullingDebugState                      : カリングの可視化用。このMeshがMesh単位のカリングでどう判定されたか(HLSL側のk_modelCullingDebugState～と同じ値。0なら見えている)
        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_skinningMatrixOffset                   = Constant::k_initialCBModelPerObjectSkinningValue;
        std::uint32_t              m_isGPUSkinning                          = Constant::k_initialCBModelPerObjectSkinningValue;
        std::uint32_t              m_cullingDebugState                      = Constant::k_initialCBModelPerObjectCullingDebugState;

        // メッシュシェーダーの中でモーフする場合に使う値(モーフを持たないMeshではm_morphDeltaBufferSRVDescriptorIndexが無効)
        // m_morphWeightBufferSRVDescriptorIndex : ウェイトのBuffer(ベイクした物か、インスタンスごとに上書きする物)
        // m_morphWeightOffset                   : このインスタンスのウェイトが始まる番号(ウェイトの番号 = m_morphWeightOffset + チャンネル番号)
        TypeAlias::DescriptorIndex m_morphVertexRangeBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_morphDeltaBufferSRVDescriptorIndex       = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_morphWeightBufferSRVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_morphWeightOffset                        = Constant::k_initialCBModelPerObjectSkinningValue;

        // 輪郭線を描く場合に使う値
        // m_outlineNormalBufferSRVDescriptorIndex : 頂点ごとの輪郭線を押し出す向き(スムーズ法線)のBuffer
        // m_padding～ : HLSL側と大きさを16byte単位で揃えるための詰め物
        TypeAlias::DescriptorIndex m_outlineNormalBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_padding                               = Constant::k_initialCBModelPerObjectPadding;
        std::uint32_t              m_padding2                              = Constant::k_initialCBModelPerObjectPadding;
        std::uint32_t              m_padding3                              = Constant::k_initialCBModelPerObjectPadding;
    };

    static_assert(sizeof(CBModelPerObject) == 224ULL, "CBModelPerObjectのサイズがHLSL側と一致しません。");
}