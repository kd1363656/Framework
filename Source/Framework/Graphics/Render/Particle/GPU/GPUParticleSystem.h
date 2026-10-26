#pragma once

namespace FWK::Graphics
{
    class Renderer;
}

namespace FWK::Graphics
{
    // GPUパーティクル(粒子の生成・移動・描画を全てGPU上で行うパーティクル)の土台
    //
    // CPUは毎フレーム「発生源(エミッター)の一覧」と「経過時間」を送るだけで、
    // 粒子1つ1つの位置や寿命はGPU上のBufferだけが持つ(CPUへは一切戻さない)
    //
    // Bufferの構成(全て作成時に最大数分を確保し、以後は増やさない)
    // ParticleBuffer          : 粒子の本体(スロット)。最大数分
    // DeadListBuffer          : 空きスロット番号の山(CounterBufferの空きの数が山の高さ)
    // AliveListBuffer × 2     : 生きているスロット番号の一覧。読み側と書き側を毎フレーム入れ替える
    // CounterBuffer           : 空きの数、生存数(読み側 / 書き側)
    // IndirectArgumentsBuffer : ExecuteIndirectが読むGroup数(GPUが自分で書く)
    //
    // 1フレームの流れ(Simulate)
    // 1. 発生源の一覧をUploadBufferへ書き、定数バッファを書く
    // 2. Emit_CS                   : 発生源ごとの数だけ粒子を作る(出す数はCPUが知っているため通常のDispatch)
    // 3. IndirectArgumentsBuild_CS : 読み側の生存数からSimulateのGroup数を書く
    // 4. Simulate_CS               : ExecuteIndirectで生存数分だけ動かし、生き残りを書き側の一覧へ詰め直す
    // 5. IndirectArgumentsBuild_CS : 書き側の生存数から描画のGroup数を書く
    // Drawは書き側の一覧をメッシュシェーダーで読み、ExecuteIndirectで描く
    class GPUParticleSystem final
    {
    private:

        // フレームリソース1つ分のCPUが書き込むBuffer
        // (GPUが前のフレームで読んでいる最中に書き換えないように、フレームリソースの数だけ用意する)
        struct FrameData final
        {
            DynamicUploadStructuredBuffer m_emitterBuffer = {};

            // 1フレーム分の定数バッファ(256byteの枡がConstant::k_gpuParticleConstantBufferSlotCount個並ぶ)
            UploadBuffer m_passConstantBuffer = {};
        };

    public:

         GPUParticleSystem() = default;
        ~GPUParticleSystem() = default;

        void BeginFrame();

        // 今フレームの発生と移動の命令をDirectCommandListへ記録する(1フレームに1回、描画より前に呼ぶ)
        // a_emitterList : m_emitOffsetが小さい順に並んだ発生源の一覧
        bool Simulate(Renderer&                                        a_renderer,
                      const std::vector<Struct::SBGPUParticleEmitter>& a_emitterList,
                      const std::uint32_t                              a_totalEmitCount,
                      const float                                      a_deltaTime);

        // 生きている粒子をExecuteIndirectで描画する(描画用PipelineStateとカメラの定数バッファを設定した後に呼ぶ)
        void Draw(const Renderer& a_renderer, const RootSignature& a_rootSignature);

        bool IsCreated() const { return m_isCreated; }

    private:

        bool Create(const Renderer& a_renderer);

        bool DispatchInitialize(Renderer& a_renderer, const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress);

        Struct::CBGPUParticlePass CreatePassConstantBuffer(const FrameData&    a_frameData,
                                                           const std::uint32_t a_emitterCount,
                                                           const std::uint32_t a_totalEmitCount,
                                                           const std::size_t   a_aliveReadListIndex,
                                                           const float         a_deltaTime) const;

        D3D12_GPU_VIRTUAL_ADDRESS WritePassConstantBuffer(const Struct::CBGPUParticlePass& a_passConstantBuffer, const std::size_t a_slotIndex, const FrameData& a_frameData) const;

        bool DispatchCompute(Renderer&                        a_renderer,
                             const Enum::PipelineStateType    a_pipelineStateType,
                             const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress,
                             const UINT                       a_threadGroupCountX) const;

        bool DispatchComputeIndirect(Renderer&                        a_renderer,
                                     const Enum::PipelineStateType    a_pipelineStateType,
                                     const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress,
                                     const UINT64&                    a_argumentBufferOffset) const;

        bool SetupComputePipeline(Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType, const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress) const;

        void TransitionForCompute(const DirectCommandList& a_directCommandList);
        void TransitionForDraw   (const DirectCommandList& a_directCommandList, const std::size_t a_drawListIndex);

        void UAVBarrierEmitWrittenBuffers(const DirectCommandList& a_directCommandList, const std::size_t a_aliveReadListIndex) const;

        std::size_t FetchVALOtherAliveListIndex(const std::size_t a_aliveListIndex) const;

        static constexpr std::size_t k_initialFrameIndex     = 0ULL;
        static constexpr std::size_t k_initialAliveListIndex = 0ULL;
        static constexpr std::size_t k_initialDrawCount      = 0ULL;
        static constexpr std::size_t k_aliveListIndexSum     = 1ULL;

        static constexpr std::uint32_t k_initialRandomSeed   = 0U;
        static constexpr std::uint32_t k_randomSeedIncrement = 1U;
        static constexpr std::uint32_t k_emptyEmitCount      = 0U;

        static constexpr UINT k_singleThreadGroupCount = 1U;
        static constexpr UINT k_emptyThreadGroupCount  = 0U;

        std::vector<FrameData> m_frameDataList = {};

        std::array<DynamicRWStructuredBuffer, Constant::k_gpuParticleAliveListBufferCount> m_aliveListBufferList = {};

        DynamicRWStructuredBuffer m_particleBuffer          = {};
        DynamicRWStructuredBuffer m_deadListBuffer          = {};
        DynamicRWStructuredBuffer m_counterBuffer           = {};
        DynamicRWStructuredBuffer m_indirectArgumentsBuffer = {};

        GPUParticleIndirectCommandSignature m_commandSignature = {};

        // m_aliveReadListIndex : 次のSimulateが読む一覧の番号(= 最後にSimulateが詰め直した一覧 = 描画する一覧)
        std::size_t m_currentFrameIndex  = k_initialFrameIndex;
        std::size_t m_aliveReadListIndex = k_initialAliveListIndex;
        std::size_t m_drawCount          = k_initialDrawCount;

        std::uint32_t m_randomSeed = k_initialRandomSeed;

        bool m_isCreated     = false;
        bool m_isInitialized = false;
    };
}