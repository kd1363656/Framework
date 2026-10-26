#pragma once

namespace FWK::Graphics
{
    class HierarchicalDepthTexture;
}

namespace FWK::Struct
{
    // GPUドリブン描画で使用する構造体
    // ※ HLSL側(Shader/Model/GPUDriven/ModelGPUDriven.hlsli)と必ず同じ並びにする

    // CPUからGPUへ送る、モデル1体分(インスタンス)の情報
    struct SBModelGPUDrivenInstance final
    {
        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        // m_morphWeightOffset : このインスタンスだけのモーフのウェイトが、上書き用ウェイトBufferの何番目から始まるか
        //                       無効値なら、ベイクしたウェイト(アニメーションのコマ番号の位置)を使う
        float         m_worldMaxScale        = Constant::k_initialGPUDrivenInstanceWorldMaxScale;
        float         m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;
        std::uint32_t m_animationFrameIndex  = Constant::k_initialGPUDrivenInstanceAnimationFrameIndex;
        std::uint32_t m_morphWeightOffset    = Constant::k_invalidGPUDrivenMorphWeightOffset;
    };

    // CPUからGPUへ送る、Mesh1つ分の情報(モデル内の全インスタンスで共通)
    struct SBModelGPUDrivenMeshInfo final
    {
        TypeAlias::Math::Vector3 m_boundsCenter = TypeAlias::Math::Vector3::Zero;
        float                    m_boundsRadius = Constant::k_initialGPUDrivenMeshInfoBoundsRadius;

        TypeAlias::DescriptorIndex m_materialBufferSRVDescriptorIndex          = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // m_lodCount : 0のMeshは、全Meshlet(m_allMeshletCount個)を元の形として描画する
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_lodOffset                              = Constant::k_initialGPUDrivenMeshInfoCount;
        std::uint32_t              m_lodCount                               = Constant::k_initialGPUDrivenMeshInfoCount;

        std::uint32_t              m_allMeshletCount                        = Constant::k_initialGPUDrivenMeshInfoCount;
        std::uint32_t              m_allTriangleCount                       = Constant::k_initialGPUDrivenMeshInfoCount;
        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_skinningPaletteBase                    = Constant::k_initialGPUDrivenMeshInfoCount;

        // スキニングするモデルの場合に使う値(スキニングしないモデルではm_skinningMatrixBufferSRVDescriptorIndexが無効)
        // スキニング行列の番号 = アニメーションのコマ番号 × m_skinningPaletteStride + m_skinningPaletteBase + ボーンパレット番号
        std::uint32_t              m_skinningPaletteStride                    = Constant::k_initialGPUDrivenMeshInfoCount;
        std::uint32_t              m_skinningFrameCount                       = Constant::k_initialGPUDrivenMeshInfoCount;
        TypeAlias::DescriptorIndex m_morphVertexRangeBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_morphDeltaBufferSRVDescriptorIndex       = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // モーフを持つMeshの場合に使う値(持たないMeshではm_morphDeltaBufferSRVDescriptorIndexが無効)
        // m_morphChannelListBufferSRVDescriptorIndex : このMeshが使うチャンネル番号の一覧(m_morphChannelListCount個)
        // m_morphWeightBufferSRVDescriptorIndex      : ベイクしたウェイト(ウェイトの番号 = コマ番号 × m_morphWeightStride + チャンネル番号)
        TypeAlias::DescriptorIndex m_morphChannelListBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_morphChannelListCount                    = Constant::k_initialGPUDrivenMeshInfoCount;
        TypeAlias::DescriptorIndex m_morphWeightBufferSRVDescriptorIndex      = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_morphWeightStride                        = Constant::k_initialGPUDrivenMeshInfoCount;

        // m_morphLODTargetErrorBufferSRVDescriptorIndex : LODごと・ターゲットごとのモーフの崩れ
        //                                                 (番号 = LOD番号 × m_morphChannelListCount + ターゲット番号)
        // m_lodSurfaceDeviationScale                    : LODで表面が実際にずれた量の倍率(ModelMeshletData::m_lodSurfaceDeviationScaleと同じ値)
        // m_lodCoveredLayerCountBufferSRVDescriptorIndex : 頂点ごとの覆われている層の数のBuffer
        // m_outlineNormalBufferSRVDescriptorIndex       : 頂点ごとの輪郭線を押し出す向き(スムーズ法線)のBuffer
        TypeAlias::DescriptorIndex m_morphLODTargetErrorBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        float                      m_lodSurfaceDeviationScale                     = Constant::k_initialGPUDrivenMeshInfoLODSurfaceDeviationScale;
        TypeAlias::DescriptorIndex m_lodCoveredLayerCountBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_outlineNormalBufferSRVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
    };

    static_assert(sizeof(SBModelGPUDrivenMeshInfo) == 112ULL, "SBModelGPUDrivenMeshInfoのサイズがHLSL側と一致しません。");

    // スキニングするモデルの、Meshごとの追加情報(ModelGPUDrivenTableBuilderへ渡す)
    struct ModelGPUDrivenSkinningInfo final
    {
        std::vector<std::uint32_t> m_paletteBaseList = {};

        std::vector<float> m_boundsRadiusList = {};

        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_morphWeightBufferSRVDescriptorIndex    = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        std::uint32_t m_paletteStride     = Constant::k_initialGPUDrivenSkinningInfoValue;
        std::uint32_t m_frameCount        = Constant::k_initialGPUDrivenSkinningInfoValue;
        std::uint32_t m_morphChannelCount = Constant::k_initialGPUDrivenSkinningInfoValue;
    };

    // 描画申請1つから、インスタンスの表へ書き込む値(ModelGPUDrivenTableBuilder::SetInstanceへ渡す)
    // ポインタは描画申請のデータを指す(SetInstanceの中で表へ写すため、呼び出しの間だけ有効であればよい)
    // m_morphWeightList : nullptr又は空なら、ベイクしたウェイトを使う
    struct ModelGPUDrivenInstanceSource final
    {
        const TypeAlias::Math::Matrix* m_worldMatrix                 = nullptr;
        const TypeAlias::Math::Matrix* m_worldInverseTransposeMatrix = nullptr;
        const std::vector<float>*      m_morphWeightList             = nullptr;

        std::uint32_t m_animationFrameIndex = Constant::k_initialGPUDrivenInstanceAnimationFrameIndex;
    };

    // 1種類のモデルをまとめた単位
    // 同じモデルのインスタンスは表の中で連続して並べ、1回のDispatchでまとめてカリングする
    struct ModelGPUDrivenBatch final
    {
        std::uint32_t m_instanceOffset = Constant::k_initialGPUDrivenBatchValue;
        std::uint32_t m_instanceCount  = Constant::k_initialGPUDrivenBatchValue;
        std::uint32_t m_meshInfoOffset = Constant::k_initialGPUDrivenBatchValue;
        std::uint32_t m_meshCount      = Constant::k_initialGPUDrivenBatchValue;
    };

    // 描画に使う表(ModelGPUDrivenTableBuilderが持ち続け、ModelGPUDrivenDrawerがGPUへ送る)
    struct ModelGPUDrivenFrameTable final
    {
        std::vector<ModelGPUDrivenBatch>      m_batchList    = {};
        std::vector<SBModelGPUDrivenInstance> m_instanceList = {};
        std::vector<SBModelGPUDrivenMeshInfo> m_meshInfoList = {};
        std::vector<ModelMeshLOD>             m_lodList      = {};

        // インスタンスごとに上書きするモーフのウェイト(SBModelGPUDrivenInstance::m_morphWeightOffsetの位置から、チャンネル数分並ぶ)
        std::vector<float> m_morphWeightList = {};

        std::size_t m_totalPairCount = Constant::k_initialGPUDrivenFrameTableTotalPairCount;

        // 表の中身を書き換えるたびに進める番号
        // ModelGPUDrivenDrawerはフレームリソースごとに書き込んだときの番号を覚え、番号が変わった表だけをGPUへ送る
        // m_meshTableVersion : m_batchList、m_meshInfoList、m_lodListの番号
        std::uint64_t m_meshTableVersion        = Constant::k_initialGPUDrivenFrameTableVersion;
        std::uint64_t m_instanceTableVersion    = Constant::k_initialGPUDrivenFrameTableVersion;
        std::uint64_t m_morphWeightTableVersion = Constant::k_initialGPUDrivenFrameTableVersion;
    };

    // 描画1回分の定数バッファ(GPUのComputeShaderが書き込み、AS / MS / PSが定数バッファとして読む)
    // 定数バッファの開始位置は256byte単位にする決まりがあるため、256byteになるようにPaddingを入れている
    struct ModelGPUDrivenDrawRecord final
    {
        CBModelPerObject m_cbModelPerObject = {};

        std::array<std::uint8_t, Constant::k_modelGPUDrivenDrawRecordByteSize - sizeof(CBModelPerObject)> m_padding = {};
    };

    static_assert(sizeof(ModelGPUDrivenDrawRecord) == Constant::k_modelGPUDrivenDrawRecordByteSize, "ModelGPUDrivenDrawRecordは256byteである必要があります。");

    // ExecuteIndirectが1回分の描画として読む引数
    // CommandSignatureで「定数バッファのアドレス → DispatchMeshの引数」の順と決めている
    struct ModelGPUDrivenIndirectCommand final
    {
        D3D12_GPU_VIRTUAL_ADDRESS m_constantBufferAddress = {};

        D3D12_DISPATCH_MESH_ARGUMENTS m_dispatchMeshArguments = {};

        std::uint32_t m_padding = Constant::k_initialGPUDrivenIndirectCommandPadding;
    };

    static_assert(sizeof(ModelGPUDrivenIndirectCommand) == Constant::k_modelGPUDrivenIndirectCommandByteSize, "ModelGPUDrivenIndirectCommandは24byteである必要があります。");

    // カリング用ComputeShaderへ渡す定数バッファ
    // HLSLの定数バッファは16byte単位で並ぶため、4つずつまとめている
    struct CBModelGPUDrivenCullingPass final
    {
        std::array<TypeAlias::Math::Vector4, Constant::k_maxGPUDrivenFrustumPlaneCount> m_frustumPlaneList = {};

        TypeAlias::Math::Vector3 m_cameraWorldPosition = TypeAlias::Math::Vector3::Zero;
        float                    m_lodProjectionScale  = Constant::k_initialGPUDrivenCullingPassFloatValue;

        float         m_lodMaxScreenSpaceErrorPixel = Constant::k_initialGPUDrivenCullingPassFloatValue;
        float         m_lodMinDistance              = Constant::k_initialGPUDrivenCullingPassFloatValue;
        std::uint32_t m_frustumPlaneCount           = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_maxDrawCount                = Constant::k_initialGPUDrivenCullingPassUIntValue;

        std::uint32_t m_instanceOffset = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_instanceCount  = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_meshInfoOffset = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_meshCount      = Constant::k_initialGPUDrivenCullingPassUIntValue;

        TypeAlias::DescriptorIndex m_instanceBufferSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshInfoBufferSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_lodBufferSRVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_drawRecordBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_indirectCommandBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_counterBufferUAVDescriptorIndex         = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_drawRecordBufferAddressLow              = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t              m_drawRecordBufferAddressHigh             = Constant::k_initialGPUDrivenCullingPassUIntValue;

        TypeAlias::Math::Matrix m_occlusionViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        Enum::ModelGPUDrivenCullingMode m_cullingMode                                = Enum::ModelGPUDrivenCullingMode::Normal;
        std::uint32_t                   m_isOcclusionEnabled                         = Constant::k_initialGPUDrivenCullingPassUIntValue;
        TypeAlias::DescriptorIndex      m_hierarchicalDepthTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t                   m_hierarchicalDepthMIPCount                  = Constant::k_initialGPUDrivenCullingPassUIntValue;

        TypeAlias::Math::Vector2   m_hierarchicalDepthSize                    = TypeAlias::Math::Vector2::Zero;
        TypeAlias::DescriptorIndex m_candidateBufferUAVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_candidateCounterBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // m_isOrthographicLOD                            : 影(平行投影)用のLOD選択をするか(1ならm_lodProjectionScaleは「1メートルあたりのテクセル数」)
        // m_minProjectedSizeTexel                        : 影の描画で、これより小さい(テクセル数)部品は描画しない
        // m_morphOverrideWeightBufferSRVDescriptorIndex : インスタンスごとに上書きするモーフのウェイトのBuffer
        std::uint32_t              m_maxCandidateCount                           = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t              m_isOrthographicLOD                           = Constant::k_initialGPUDrivenCullingPassUIntValue;
        float                      m_minProjectedSizeTexel                       = Constant::k_initialGPUDrivenCullingPassFloatValue;
        TypeAlias::DescriptorIndex m_morphOverrideWeightBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // m_isCullingDebugVisualize : 1なら視錐台の外、又は隠れていると判定した組み合わせも、判定結果を付けて描画コマンドを書く(カリングの可視化)
        std::uint32_t m_isCullingDebugVisualize = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_padding0                = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_padding1                = Constant::k_initialGPUDrivenCullingPassUIntValue;
        std::uint32_t m_padding2                = Constant::k_initialGPUDrivenCullingPassUIntValue;
    };

    static_assert(sizeof(CBModelGPUDrivenCullingPass) == Constant::k_cbModelGPUDrivenCullingPassByteSize, "CBModelGPUDrivenCullingPassのサイズがHLSL側の定数バッファと一致しません。");

    // オクルージョンカリングの候補(1回目で隠れていると判定された「インスタンス × Mesh」の組み合わせ)
    struct ModelGPUDrivenOcclusionCandidate final
    {
        std::uint32_t m_instanceIndex = Constant::k_initialGPUDrivenOcclusionCandidateIndex;
        std::uint32_t m_meshInfoIndex = Constant::k_initialGPUDrivenOcclusionCandidateIndex;
    };

    // カリングに使う「見る範囲」1つ分(カメラ1つ、又は影のカスケード1枚)
    struct ModelGPUDrivenCullingView final
    {
        // 視錐台を作る平面(xyz : 内側を向いた法線、w : 原点からの距離)
        std::array<TypeAlias::Math::Vector4, Constant::k_maxGPUDrivenFrustumPlaneCount> m_frustumPlaneList = {};

        // 判定に使う平面の数(影はライトより手前の物も影を落とすため、手前の平面を使わず5枚にする)
        std::uint32_t m_frustumPlaneCount = Constant::k_initialGPUDrivenFrustumPlaneCount;

        Enum::ModelGPUDrivenCullingMode m_cullingMode = Enum::ModelGPUDrivenCullingMode::Normal;

        // オクルージョンカリングに使うHi-Zと、その判定に使うビュープロジェクション行列
        // Hi-Zがまだ無い場合(最初のフレームなど)はnullptrにする
        const Graphics::HierarchicalDepthTexture* m_hierarchicalDepthTexture = nullptr;

        TypeAlias::Math::Matrix m_occlusionViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        std::size_t m_candidateSourceViewIndex = Constant::k_initialGPUDrivenCandidateSourceViewIndex;

        float m_orthographicTexelPerWorldUnit = Constant::k_initialGPUDrivenOrthographicTexelPerWorldUnit;

        bool m_isOrthographicLOD       = false;
        bool m_isCullingDebugVisualize = false;
    };
}