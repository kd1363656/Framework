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
        static constexpr float k_initialWorldMaxScale = 1.0F;

        static constexpr std::uint32_t k_initialAnimationFrameIndex = 0U;
        static constexpr std::uint32_t k_initialPadding             = 0U;

        TypeAlias::Math::Matrix m_worldMatrix                 = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_worldInverseTransposeMatrix = TypeAlias::Math::Matrix::Identity;

        // m_animationFrameIndex : スキニングするモデルの場合に使う、事前計算したアニメーションの何コマ目か
        float         m_worldMaxScale        = k_initialWorldMaxScale;
        float         m_worldOrientationSign = Constant::k_normalModelWorldOrientationSign;
        std::uint32_t m_animationFrameIndex  = k_initialAnimationFrameIndex;
        std::uint32_t m_padding              = k_initialPadding;
    };

    // CPUからGPUへ送る、Mesh1つ分の情報(モデル内の全インスタンスで共通)
    struct SBModelGPUDrivenMeshInfo final
    {
        static constexpr float k_initialBoundsRadius = 0.0F;

        static constexpr std::uint32_t k_initialCount   = 0U;
        static constexpr std::uint32_t k_initialPadding = 0U;

        TypeAlias::Math::Vector3 m_boundsCenter = TypeAlias::Math::Vector3::Zero;
        float                    m_boundsRadius = k_initialBoundsRadius;

        // m_materialBufferSRVDescriptorIndex : MaterialのGPUDataBuffer
        // Materialの値(色やテクスチャ)はMaterialの種類ごとに違うため、ここには持たせずMaterial側のBufferを参照する
        TypeAlias::DescriptorIndex m_materialBufferSRVDescriptorIndex          = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_vertexBufferSRVDescriptorIndex            = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBufferSRVDescriptorIndex           = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_uniqueVertexIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // m_lodCount : 0のMeshは、全Meshlet(m_allMeshletCount個)を元の形として描画する
        TypeAlias::DescriptorIndex m_primitiveIndexBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshletBoundsBufferSRVDescriptorIndex  = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_lodOffset                              = k_initialCount;
        std::uint32_t              m_lodCount                               = k_initialCount;

        std::uint32_t              m_allMeshletCount                        = k_initialCount;
        std::uint32_t              m_allTriangleCount                       = k_initialCount;
        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_skinningPaletteBase                    = k_initialCount;

        // スキニングするモデルの場合に使う値(スキニングしないモデルではm_skinningMatrixBufferSRVDescriptorIndexが無効)
        // スキニング行列の番号 = アニメーションのコマ番号 × m_skinningPaletteStride + m_skinningPaletteBase + ボーンパレット番号
        std::uint32_t m_skinningPaletteStride = k_initialCount;
        std::uint32_t m_skinningFrameCount    = k_initialCount;
        std::uint32_t m_padding               = k_initialPadding;
        std::uint32_t m_padding2              = k_initialPadding;
    };

    static_assert(sizeof(SBModelGPUDrivenMeshInfo) == 80ULL, "SBModelGPUDrivenMeshInfoのサイズがHLSL側と一致しません。");

    // スキニングするモデルの、Meshごとの追加情報(ModelGPUDrivenTableBuilderへ渡す)
    struct ModelGPUDrivenSkinningInfo final
    {
        static constexpr std::uint32_t k_initialValue = 0U;

        // Meshごとの、スキニング行列の並びの中での開始位置
        std::vector<std::uint32_t> m_paletteBaseList = {};

        // Meshごとの、アニメーションで動いても全頂点を囲める球の半径(カリング用)
        std::vector<float> m_boundsRadiusList = {};

        TypeAlias::DescriptorIndex m_skinningMatrixBufferSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // 1コマ分のスキニング行列の数(全Meshのボーンパレットの合計)と、コマの総数
        std::uint32_t m_paletteStride = k_initialValue;
        std::uint32_t m_frameCount    = k_initialValue;
    };

    // 1種類のモデルをまとめた単位
    // 同じモデルのインスタンスは表の中で連続して並べ、1回のDispatchでまとめてカリングする
    struct ModelGPUDrivenBatch final
    {
        static constexpr std::uint32_t k_initialValue = 0U;

        std::uint32_t m_instanceOffset = k_initialValue;
        std::uint32_t m_instanceCount  = k_initialValue;
        std::uint32_t m_meshInfoOffset = k_initialValue;
        std::uint32_t m_meshCount      = k_initialValue;
    };

    // 1フレーム分の表(描画申請から作り、ModelGPUDrivenDrawerがGPUへ送る)
    struct ModelGPUDrivenFrameTable final
    {
        static constexpr std::size_t k_initialTotalPairCount = 0ULL;

        std::vector<ModelGPUDrivenBatch>      m_batchList    = {};
        std::vector<SBModelGPUDrivenInstance> m_instanceList = {};
        std::vector<SBModelGPUDrivenMeshInfo> m_meshInfoList = {};
        std::vector<ModelMeshLOD>             m_lodList      = {};

        // 「インスタンス × Mesh」の組み合わせの総数
        std::size_t m_totalPairCount = k_initialTotalPairCount;
    };

    // 描画1回分の定数バッファ(GPUのComputeShaderが書き込み、AS / MS / PSが定数バッファとして読む)
    // 定数バッファの開始位置は256byte単位にする決まりがあるため、256byteになるようにPaddingを入れている
    struct ModelGPUDrivenDrawRecord final
    {
        static constexpr std::size_t k_byteSize = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;

        CBModelPerObject m_cbModelPerObject = {};

        std::array<std::uint8_t, k_byteSize - sizeof(CBModelPerObject)> m_padding = {};
    };

    static_assert(sizeof(ModelGPUDrivenDrawRecord) == ModelGPUDrivenDrawRecord::k_byteSize, "ModelGPUDrivenDrawRecordは256byteである必要があります。");

    // ExecuteIndirectが1回分の描画として読む引数
    // CommandSignatureで「定数バッファのアドレス → DispatchMeshの引数」の順と決めている
    struct ModelGPUDrivenIndirectCommand final
    {
        static constexpr std::size_t k_byteSize = 24ULL;

        static constexpr std::uint32_t k_initialPadding = 0U;

        D3D12_GPU_VIRTUAL_ADDRESS m_constantBufferAddress = {};

        D3D12_DISPATCH_MESH_ARGUMENTS m_dispatchMeshArguments = {};

        std::uint32_t m_padding = k_initialPadding;
    };

    static_assert(sizeof(ModelGPUDrivenIndirectCommand) == ModelGPUDrivenIndirectCommand::k_byteSize, "ModelGPUDrivenIndirectCommandは24byteである必要があります。");

    // カリング用ComputeShaderへ渡す定数バッファ
    // HLSLの定数バッファは16byte単位で並ぶため、4つずつまとめている
    struct CBModelGPUDrivenCullingPass final
    {
        // HLSL側の定数バッファのサイズ
        static constexpr std::size_t k_byteSize = 288ULL;

        static constexpr std::size_t k_maxFrustumPlaneCount = 6ULL;

        static constexpr float k_initialFloatValue = 0.0F;

        static constexpr std::uint32_t k_initialUIntValue = 0U;

        std::array<TypeAlias::Math::Vector4, k_maxFrustumPlaneCount> m_frustumPlaneList = {};

        TypeAlias::Math::Vector3 m_cameraWorldPosition = TypeAlias::Math::Vector3::Zero;
        float                    m_lodProjectionScale  = k_initialFloatValue;

        float         m_lodMaxScreenSpaceErrorPixel = k_initialFloatValue;
        float         m_lodMinDistance              = k_initialFloatValue;
        std::uint32_t m_frustumPlaneCount           = k_initialUIntValue;
        std::uint32_t m_maxDrawCount                = k_initialUIntValue;

        std::uint32_t m_instanceOffset = k_initialUIntValue;
        std::uint32_t m_instanceCount  = k_initialUIntValue;
        std::uint32_t m_meshInfoOffset = k_initialUIntValue;
        std::uint32_t m_meshCount      = k_initialUIntValue;

        TypeAlias::DescriptorIndex m_instanceBufferSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_meshInfoBufferSRVDescriptorIndex   = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_lodBufferSRVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_drawRecordBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        TypeAlias::DescriptorIndex m_indirectCommandBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_counterBufferUAVDescriptorIndex         = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t              m_drawRecordBufferAddressLow              = k_initialUIntValue;
        std::uint32_t              m_drawRecordBufferAddressHigh             = k_initialUIntValue;

        // オクルージョンカリングで使うビュープロジェクション行列(1回目は前のフレーム、2回目は今のフレーム)
        TypeAlias::Math::Matrix m_occlusionViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        Enum::ModelGPUDrivenCullingMode m_cullingMode                                = Enum::ModelGPUDrivenCullingMode::Normal;
        std::uint32_t                   m_isOcclusionEnabled                         = k_initialUIntValue;
        TypeAlias::DescriptorIndex      m_hierarchicalDepthTextureSRVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        std::uint32_t                   m_hierarchicalDepthMIPCount                  = k_initialUIntValue;

        TypeAlias::Math::Vector2   m_hierarchicalDepthSize                    = TypeAlias::Math::Vector2::Zero;
        TypeAlias::DescriptorIndex m_candidateBufferUAVDescriptorIndex        = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_candidateCounterBufferUAVDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;

        // m_isOrthographicLOD     : 影(平行投影)用のLOD選択をするか(1ならm_lodProjectionScaleは「1メートルあたりのテクセル数」)
        // m_minProjectedSizeTexel : 影の描画で、これより小さい(テクセル数)部品は描画しない
        std::uint32_t m_maxCandidateCount     = k_initialUIntValue;
        std::uint32_t m_isOrthographicLOD     = k_initialUIntValue;
        float         m_minProjectedSizeTexel = k_initialFloatValue;
        std::uint32_t m_padding               = k_initialUIntValue;
    };

    static_assert(sizeof(CBModelGPUDrivenCullingPass) == CBModelGPUDrivenCullingPass::k_byteSize, "CBModelGPUDrivenCullingPassのサイズがHLSL側の定数バッファと一致しません。");

    // オクルージョンカリングの候補(1回目で隠れていると判定された「インスタンス × Mesh」の組み合わせ)
    struct ModelGPUDrivenOcclusionCandidate final
    {
        static constexpr std::uint32_t k_initialIndex = 0U;

        std::uint32_t m_instanceIndex = k_initialIndex;
        std::uint32_t m_meshInfoIndex = k_initialIndex;
    };

    // カリングに使う「見る範囲」1つ分(カメラ1つ、又は影のカスケード1枚)
    struct ModelGPUDrivenCullingView final
    {
        static constexpr std::size_t k_initialCandidateSourceViewIndex = 0ULL;

        static constexpr float k_initialOrthographicTexelPerWorldUnit = 0.0F;

        static constexpr std::uint32_t k_initialFrustumPlaneCount = 0U;

        // 視錐台を作る平面(xyz : 内側を向いた法線、w : 原点からの距離)
        std::array<TypeAlias::Math::Vector4, CBModelGPUDrivenCullingPass::k_maxFrustumPlaneCount> m_frustumPlaneList = {};

        // 判定に使う平面の数(影はライトより手前の物も影を落とすため、手前の平面を使わず5枚にする)
        std::uint32_t m_frustumPlaneCount = k_initialFrustumPlaneCount;

        // カリングの種類(オクルージョンカリングを行う場合はEarly又はLate)
        Enum::ModelGPUDrivenCullingMode m_cullingMode = Enum::ModelGPUDrivenCullingMode::Normal;

        // オクルージョンカリングに使うHi-Zと、その判定に使うビュープロジェクション行列
        // Hi-Zがまだ無い場合(最初のフレームなど)はnullptrにする
        const Graphics::HierarchicalDepthTexture* m_hierarchicalDepthTexture = nullptr;

        TypeAlias::Math::Matrix m_occlusionViewProjectionMatrix = TypeAlias::Math::Matrix::Identity;

        // Lateの場合に、候補を読む元のView(Earlyを行ったView)の番号
        std::size_t m_candidateSourceViewIndex = k_initialCandidateSourceViewIndex;

        // 影(平行投影)用のLOD選択の設定
        // m_orthographicTexelPerWorldUnit : シャドウマップ上で1メートルが何テクセルになるか
        //                                   (カスケードが遠いほど1テクセルが大きく、粗いLODでも見た目が変わらない)
        float m_orthographicTexelPerWorldUnit = k_initialOrthographicTexelPerWorldUnit;

        // trueなら、カメラからの距離ではなくシャドウマップのテクセルを基準にLODを選ぶ
        bool m_isOrthographicLOD = false;
    };
}