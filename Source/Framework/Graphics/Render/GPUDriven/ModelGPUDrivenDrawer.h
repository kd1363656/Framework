#pragma once

namespace FWK::Graphics
{
    // モデルをGPUドリブン(GPU主導)で描画するクラス(StaticModel、SkeletalAnimationModel共通)
    //
    // 従来 : CPUが「全モデル × 全Mesh」をループし、1つずつ定数バッファを作ってDispatchMeshしていた
    //        → 500体 × 22Mesh × 5パス = 約5.5万回の処理がCPUで毎フレーム発生していた
    //
    // 今回 : CPUは「モデル1体ごとのワールド行列」と「Meshの情報」を表にしてGPUへ送るだけにする
    //        1. ComputeShaderが「インスタンス × Mesh」の組み合わせを並列に調べ、
    //           見える物だけについて定数バッファと描画コマンドをGPU上のBufferへ書き込む(カリング + LOD選択)
    //        2. ExecuteIndirectで、GPU上の描画コマンドを1回の命令でまとめて実行する
    //        → CPUの処理量がモデル数にほとんど影響されなくなる
    //
    // 表(インスタンス、Mesh、LOD)の作り方はモデルの種類ごとに異なるため、
    // 表は各描画申請クラスのTableBuilderが持ち続け、このクラスは変わった表だけをGPUへ送る
    //
    // 「見る範囲(View)」はカメラ1つ、又は影のカスケード1枚ごとに用意し、Viewごとに別々のBufferへ書き込む
    class ModelGPUDrivenDrawer final
    {
    private:

        // フレームリソース1つ分のデータ
        // CPUが書き込むBufferは、GPUが前のフレームで読んでいる最中に書き換えないようにフレームリソースの数だけ用意する
        struct FrameData final
        {
            DynamicUploadStructuredBuffer m_instanceBuffer    = {};
            DynamicUploadStructuredBuffer m_meshInfoBuffer    = {};
            DynamicUploadStructuredBuffer m_lodBuffer         = {};
            DynamicUploadStructuredBuffer m_morphWeightBuffer = {};

            // カリング用ComputeShaderへ渡す定数バッファ(Dispatch1回につき256byteずつ使う)
            UploadBuffer m_cullingPassConstantBuffer = {};

            // どのViewのカウンターを書き込んだか(1ビットが1つのViewを表す)
            // 描画しなかったViewの古い値を読まないようにするため
            // 最後に書き込んだ表の番号(ModelGPUDrivenFrameTableのVersionと比べ、変わった表だけを書き込む)
            std::uint64_t m_uploadedMeshTableVersion        = k_notUploadedTableVersion;
            std::uint64_t m_uploadedInstanceTableVersion    = k_notUploadedTableVersion;
            std::uint64_t m_uploadedMorphWeightTableVersion = k_notUploadedTableVersion;

        };

        // View(カメラ又は影のカスケード)1つ分のGPU上のBuffer
        struct ViewData final
        {
            DynamicRWStructuredBuffer m_drawRecordBuffer      = {};
            DynamicRWStructuredBuffer m_indirectCommandBuffer = {};
            DynamicRWStructuredBuffer m_counterBuffer         = {};

            // オクルージョンカリング1回目で「隠れていた」組み合わせを記録するBuffer(1回目を行うViewだけ作成する)
            DynamicRWStructuredBuffer m_candidateBuffer = {};

            bool m_isCulled = false;
        };

    public:

         ModelGPUDrivenDrawer() = default;
        ~ModelGPUDrivenDrawer() = default;

        void BeginFrame();

        // TableBuilderの表(インスタンス、Mesh、LOD)のうち、変わった物だけをこのフレームで使うBufferへ書き込む
        // 1フレームに1回、最初のカリングの前に呼ぶ(IsTableUploadRequiredで呼ぶ必要があるか確認する)
        void UploadFrameTable(const Renderer& a_renderer, const Struct::ModelGPUDrivenFrameTable& a_frameTable);

        bool DispatchCulling(Renderer& a_renderer, const std::size_t a_viewIndex, const Struct::ModelGPUDrivenCullingView& a_cullingView);

        void ExecuteIndirect(const Renderer& a_renderer, const RootSignature& a_rootSignature, const std::size_t a_viewIndex);

        bool IsTableUploadRequired() const { return m_isTableUploadRequired; }

        // ビュープロジェクション行列から、視錐台を作る6枚の平面を求める
        // a_useNearPlane : 手前の平面で判定するか(影はライトより手前の物も影を落とすため使わない)
        static Struct::ModelGPUDrivenCullingView BuildCullingView(const TypeAlias::Math::Matrix& a_viewProjectionMatrix, const bool a_useNearPlane);

    private:

        bool Create(const Renderer& a_renderer);

        bool CreateViewData(ViewData& a_viewData) const;

        Struct::CBModelGPUDrivenCullingPass CreateCullingPass(const Struct::ModelGPUDrivenBatch&       a_modelBatch,
                                                              const Struct::ModelGPUDrivenCullingView& a_cullingView,
                                                              const ModelLODSelector&                  a_modelLODSelector,
                                                              const FrameData&                         a_frameData,
                                                              const ViewData&                          a_viewData,
                                                              const ViewData&                          a_candidateViewData) const;

        void WriteCullingPassAndDispatch(const DirectCommandList&                   a_directCommandList,
                                         const RootSignature&                       a_rootSignature,
                                         const Struct::CBModelGPUDrivenCullingPass& a_cullingPass,
                                         const UINT                                 a_threadCount);

        static constexpr std::size_t k_maxDrawCountPerView             = 32768ULL;
        static constexpr std::size_t k_maxCullingDispatchCountPerFrame = 256ULL;
        static constexpr std::size_t k_maxViewCount                    = 8ULL;

        static constexpr std::size_t k_counterByteSize = sizeof(std::uint32_t) * Constant::k_modelGPUDrivenCounterElementCount;

        // カリング用定数バッファ1つ分の間隔(定数バッファの配置単位256byteの倍数で、CBModelGPUDrivenCullingPassが入る大きさ)
        static constexpr std::size_t k_cullingPassConstantBufferStride = 512ULL;

        static_assert(k_cullingPassConstantBufferStride % D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT == 0ULL, "定数バッファの間隔は256byteの倍数である必要があります。");
        static_assert(k_cullingPassConstantBufferStride >= sizeof(Struct::CBModelGPUDrivenCullingPass),        "定数バッファの間隔がCBModelGPUDrivenCullingPassより小さくなっています。");

        static constexpr std::size_t k_initialSize = 0ULL;

        static constexpr std::size_t k_frustumLeftPlaneIndex   = 0ULL;
        static constexpr std::size_t k_frustumRightPlaneIndex  = 1ULL;
        static constexpr std::size_t k_frustumBottomPlaneIndex = 2ULL;
        static constexpr std::size_t k_frustumTopPlaneIndex    = 3ULL;
        static constexpr std::size_t k_frustumFarPlaneIndex    = 4ULL;
        static constexpr std::size_t k_frustumNearPlaneIndex   = 5ULL;

        static constexpr float k_minPlaneNormalLength = 0.0F;

        static constexpr float k_shadowMaxErrorTexel = 1.0F;

        static constexpr float k_shadowMinCasterSizeTexel = 1.0F;

        static constexpr std::uint64_t k_notUploadedTableVersion = std::numeric_limits<std::uint64_t>::max();

        static constexpr int k_zeroFillByte = 0;

        static constexpr UINT64 k_bufferStartOffset             = 0ULL;
        static constexpr UINT64 k_addressHighShiftBit           = 32ULL;
        static constexpr UINT64 k_addressLowMask                = 0xFFFFFFFFULL;

        static constexpr std::uint32_t k_frustumPlaneCountWithNear    = 6U;
        static constexpr std::uint32_t k_frustumPlaneCountWithoutNear = 5U;
        static constexpr std::uint32_t k_trueValue                    = 1U;

        static constexpr UINT k_singleThreadGroupCount  = 1U;
        static constexpr UINT k_initialThreadGroupCount = 0U;

        std::vector<FrameData> m_frameDataList = {};
        std::vector<ViewData>  m_viewDataList  = {};

        std::vector<ModelIndirectDispatchMeshCommandSignature> m_commandSignatureList = {};

        std::vector<Struct::ModelGPUDrivenBatch> m_modelBatchList = {};

        // カウンターを0に戻すためのコピー元(中身は常に0)
        UploadBuffer m_zeroCounterUploadBuffer = {};

        std::size_t m_currentFrameIndex    = k_initialSize;
        std::size_t m_cullingDispatchCount = k_initialSize;

        std::size_t m_totalPairCount = k_initialSize;

        std::size_t m_instanceCount = k_initialSize;

        bool m_isCreated             = false;
        bool m_isTableUploadRequired = true;
    };
}