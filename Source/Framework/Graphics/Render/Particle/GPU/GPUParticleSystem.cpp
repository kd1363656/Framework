#include "GPUParticleSystem.h"

void FWK::Graphics::GPUParticleSystem::BeginFrame()
{
    // 描画用の定数バッファの枡は、フレームの最初の描画から順番に使い直す
    m_drawCount = k_initialDrawCount;
}

bool FWK::Graphics::GPUParticleSystem::Simulate(Renderer&                                        a_renderer,
                                                const std::vector<Struct::SBGPUParticleEmitter>& a_emitterList,
                                                const std::uint32_t                              a_totalEmitCount,
                                                const float                                      a_deltaTime)
{
    FWK_ASSERT_RETURN_VALUE_IF(a_emitterList.size() > Constant::k_gpuParticleMaxEmitterCount,     "発生源の数がGPUパーティクルの最大数を超えているため、Simulateに失敗しました。",       false);
    FWK_ASSERT_RETURN_VALUE_IF(a_totalEmitCount      > Constant::k_gpuParticleMaxEmitCountPerFrame, "今フレームに出す粒子の数がGPUパーティクルの最大数を超えているため、Simulateに失敗しました。", false);

    // GPU上のBufferは、初めて使うときに作成する
    if (!m_isCreated)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!Create(a_renderer), "GPUパーティクル用Bufferの作成に失敗しました。", false);
    }

    m_currentFrameIndex = a_renderer.GetREFCurrentFrameResourceIndex();

    FWK_ASSERT_RETURN_VALUE_IF(m_currentFrameIndex >= m_frameDataList.size(), "現在のフレームリソース番号がGPUパーティクルのFrameData数を超えています。", false);

    auto& l_frameData         = m_frameDataList[m_currentFrameIndex];
    auto& l_directCommandList = a_renderer.GetMutableREFDirectCommandList();

    // 1. 発生源の一覧をGPUが読めるBufferへ書き、1フレーム分の設定を定数バッファへ書く
    //    (IndirectArgumentsBuild_CSは「どちらのGroup数を書くか」だけが違うため、その値だけを変えた枡を別に書く)
    FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_emitterBuffer.Write(a_emitterList), "発生源の一覧の書き込みに失敗しました。", false);

    const auto l_aliveWriteListIndex = FetchVALOtherAliveListIndex(m_aliveReadListIndex);
    const auto l_emitterCount        = static_cast<std::uint32_t>(a_emitterList.size());

    auto l_passConstantBuffer = CreatePassConstantBuffer(l_frameData,
                                                         l_emitterCount,
                                                         a_totalEmitCount,
                                                         m_aliveReadListIndex,
                                                         a_deltaTime);

    const auto l_commonConstantBufferAddress = WritePassConstantBuffer(l_passConstantBuffer, Constant::k_gpuParticleCommonConstantBufferSlotIndex, l_frameData);

    l_passConstantBuffer.m_indirectArgumentsBuildTarget = Enum::GPUParticleIndirectArgumentsBuildTarget::Simulate;

    const auto l_simulateArgumentsConstantBufferAddress = WritePassConstantBuffer(l_passConstantBuffer, Constant::k_gpuParticleSimulateArgumentsConstantBufferSlotIndex, l_frameData);

    l_passConstantBuffer.m_indirectArgumentsBuildTarget = Enum::GPUParticleIndirectArgumentsBuildTarget::Draw;

    const auto l_drawArgumentsConstantBufferAddress = WritePassConstantBuffer(l_passConstantBuffer, Constant::k_gpuParticleDrawArgumentsConstantBufferSlotIndex, l_frameData);

    FWK_ASSERT_RETURN_VALUE_IF(l_commonConstantBufferAddress            == Constant::k_invalidDynamicBufferGPUVirtualAddress ||
                               l_simulateArgumentsConstantBufferAddress == Constant::k_invalidDynamicBufferGPUVirtualAddress ||
                               l_drawArgumentsConstantBufferAddress     == Constant::k_invalidDynamicBufferGPUVirtualAddress,
                               "GPUパーティクル用定数バッファの書き込みに失敗しました。",
                               false);

    // 2. ComputeShaderから読み書きできる状態(UNORDERED_ACCESS)へ全てのBufferを遷移する
    TransitionForCompute(l_directCommandList);

    // 最初の1回だけ、全スロットを空きとしてDeadListへ積む
    if (!m_isInitialized)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!DispatchInitialize(a_renderer, l_commonConstantBufferAddress), "GPUパーティクルの初期化Dispatchに失敗しました。", false);

        m_isInitialized = true;
    }

    // 3. 発生 : 今フレームに出す粒子の数だけThreadを起動する(出す粒子が無ければ飛ばす)
    if (a_totalEmitCount != k_emptyEmitCount)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!DispatchCompute(a_renderer,
                                                    Enum::PipelineStateType::GPUParticleEmit,
                                                    l_commonConstantBufferAddress,
                                                    Utility::DivideRoundUp(a_totalEmitCount, Constant::k_gpuParticleEmitThreadCount)),
                                                    "GPUパーティクルの発生Dispatchに失敗しました。",
                                                    false);

        // 発生が書き終わってから、後続のComputeShaderが同じBufferを読むようにする
        UAVBarrierEmitWrittenBuffers(l_directCommandList, m_aliveReadListIndex);
    }

    // 4. 読み側の生存数からSimulateのGroup数をGPUに書かせ、書き側の生存数を0に戻す
    FWK_ASSERT_RETURN_VALUE_IF(!DispatchCompute(a_renderer,
                                                Enum::PipelineStateType::GPUParticleIndirectArgumentsBuild,
                                                l_simulateArgumentsConstantBufferAddress,
                                                k_singleThreadGroupCount),
                                                "GPUパーティクルのSimulate用Group数の書き込みDispatchに失敗しました。",
                                                false);

    // Group数の書き込みが終わってからExecuteIndirectが読めるように、
    // IndirectArgumentsBufferをINDIRECT_ARGUMENTへ遷移する(0に戻した生存数も、Simulateが読む前に書き終わらせる)
    l_directCommandList.UAVResourceBarrier(m_counterBuffer.GetREFBufferGPUResource().m_resource);

    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, m_indirectArgumentsBuffer, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

    // 5. 移動 : GPUが書いたGroup数で生きている粒子だけを動かす
    FWK_ASSERT_RETURN_VALUE_IF(!DispatchComputeIndirect(a_renderer,
                                                        Enum::PipelineStateType::GPUParticleSimulate,
                                                        l_commonConstantBufferAddress,
                                                        Constant::k_gpuParticleSimulateArgumentsByteOffset),
                                                        "GPUパーティクルの移動Dispatchに失敗しました。",
                                                        false);

    // 6. 書き側の生存数から描画のGroup数をGPUに書かせる
    l_directCommandList.UAVResourceBarrier(m_counterBuffer.GetREFBufferGPUResource().m_resource);

    Utility::TransitionDynamicRWStructuredBuffer(l_directCommandList, m_indirectArgumentsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    FWK_ASSERT_RETURN_VALUE_IF(!DispatchCompute(a_renderer,
                                                Enum::PipelineStateType::GPUParticleIndirectArgumentsBuild,
                                                l_drawArgumentsConstantBufferAddress,
                                                k_singleThreadGroupCount),
                                                "GPUパーティクルの描画用Group数の書き込みDispatchに失敗しました。",
                                                false);

    // 7. 描画(メッシュシェーダー)が読める状態へ遷移する
    TransitionForDraw(l_directCommandList, l_aliveWriteListIndex);

    // 次のフレームは、今フレームに詰め直した一覧を読む
    m_aliveReadListIndex = l_aliveWriteListIndex;

    m_randomSeed += k_randomSeedIncrement;

    return true;
}

void FWK::Graphics::GPUParticleSystem::Draw(const Renderer& a_renderer, const RootSignature& a_rootSignature)
{
    // 一度もSimulateしていなければ、描く粒子も描ける状態のBufferも無い
    if (!m_isInitialized) { return; }

    FWK_ASSERT_RETURN_IF(m_drawCount >= Constant::k_gpuParticleDrawConstantBufferSlotCount, "1フレームの描画回数がGPUパーティクルの定数バッファの枡の数を超えています。");
    FWK_ASSERT_RETURN_IF(m_currentFrameIndex >= m_frameDataList.size(),                     "現在のフレームリソース番号がGPUパーティクルのFrameData数を超えています。");

    const auto& l_frameData         = m_frameDataList[m_currentFrameIndex];
    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    // 描画する一覧は「最後にSimulateが詰め直した一覧」(= 次のSimulateが読む一覧)
    // 定数バッファの書き側の値として渡すため、読み側にはもう一方の番号を入れる
    const auto& l_passConstantBuffer = CreatePassConstantBuffer(l_frameData,
                                                                k_emptyEmitCount,
                                                                k_emptyEmitCount,
                                                                FetchVALOtherAliveListIndex(m_aliveReadListIndex),
                                                                Constant::k_initialCBGPUParticlePassFloatValue);

    const auto l_slotIndex                 = Constant::k_gpuParticleComputeConstantBufferSlotCount + m_drawCount;
    const auto l_passConstantBufferAddress = WritePassConstantBuffer(l_passConstantBuffer, l_slotIndex, l_frameData);

    FWK_ASSERT_RETURN_IF(l_passConstantBufferAddress == Constant::k_invalidDynamicBufferGPUVirtualAddress, "GPUパーティクルの描画用定数バッファの書き込みに失敗しました。");

    l_directCommandList.SetupConstantBufferView(l_passConstantBufferAddress, a_rootSignature, Enum::RootParameterType::CBGPUParticlePass);

    // GPUが書いたGroup数でメッシュシェーダーを起動する
    l_directCommandList.ExecuteIndirectSingleCommand(m_commandSignature.GetREFDispatchMeshCommandSignature(),
                                                     m_indirectArgumentsBuffer.GetREFBufferGPUResource().m_resource,
                                                     Constant::k_gpuParticleDrawArgumentsByteOffset);

    ++m_drawCount;
}

bool FWK::Graphics::GPUParticleSystem::Create(const Renderer& a_renderer)
{
    auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
    auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
    auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    const auto& l_device             = l_graphicsManager.GetREFDevice            ();
    const auto& l_gpuMemoryAllocator = l_resourceContext.GetREFGPUMemoryAllocator();
    const auto  l_frameResourceCount = a_renderer.GetREFFrameResourceList        ().size();

    FWK_ASSERT_RETURN_VALUE_IF(l_frameResourceCount == k_initialFrameIndex, "フレームリソースが無いため、GPUパーティクル用Bufferの作成に失敗しました。", false);

    m_frameDataList.clear ();
    m_frameDataList.resize(l_frameResourceCount);

    for (auto& l_frameData : m_frameDataList)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_emitterBuffer.Create<Struct::SBGPUParticleEmitter>(l_device, Constant::k_gpuParticleMaxEmitterCount, l_cbvSRVUAVDescriptorPool), "発生源用Bufferの作成に失敗しました。",                 false);
        FWK_ASSERT_RETURN_VALUE_IF(!l_frameData.m_passConstantBuffer.Create(l_device, Constant::k_gpuParticleConstantBufferSlotCount * Constant::k_gpuParticleConstantBufferSlotStride),  "GPUパーティクル用定数バッファの作成に失敗しました。", false);
    }

    FWK_ASSERT_RETURN_VALUE_IF(!m_particleBuffer.Create<Struct::SBGPUParticle>(l_device,
                                                                               l_gpuMemoryAllocator,
                                                                               Constant::k_gpuParticleMaxCount,
                                                                               l_cbvSRVUAVDescriptorPool),
                                                                               "粒子用Bufferの作成に失敗しました。",
                                                                               false);

    FWK_ASSERT_RETURN_VALUE_IF(!m_deadListBuffer.Create<std::uint32_t>(l_device,
                                                                       l_gpuMemoryAllocator,
                                                                       Constant::k_gpuParticleMaxCount,
                                                                       l_cbvSRVUAVDescriptorPool),
                                                                       "空きスロット一覧用Bufferの作成に失敗しました。",
                                                                       false);

    for (auto& l_aliveListBuffer : m_aliveListBufferList)
    {
        FWK_ASSERT_RETURN_VALUE_IF(!l_aliveListBuffer.Create<std::uint32_t>(l_device,
                                                                            l_gpuMemoryAllocator,
                                                                            Constant::k_gpuParticleMaxCount,
                                                                            l_cbvSRVUAVDescriptorPool),
                                                                            "生存スロット一覧用Bufferの作成に失敗しました。",
                                                                            false);
    }

    FWK_ASSERT_RETURN_VALUE_IF(!m_counterBuffer.Create<std::uint32_t>(l_device,
                                                                      l_gpuMemoryAllocator,
                                                                      Constant::k_gpuParticleCounterElementCount,
                                                                      l_cbvSRVUAVDescriptorPool),
                                                                      "カウンター用Bufferの作成に失敗しました。",
                                                                      false);

    FWK_ASSERT_RETURN_VALUE_IF(!m_indirectArgumentsBuffer.Create<Struct::GPUParticleIndirectArguments>(l_device,
                                                                                                       l_gpuMemoryAllocator,
                                                                                                       Constant::k_gpuParticleIndirectArgumentsElementCount,
                                                                                                       l_cbvSRVUAVDescriptorPool),
                                                                                                       "ExecuteIndirectの引数用Bufferの作成に失敗しました。",
                                                                                                       false);

    FWK_ASSERT_RETURN_VALUE_IF(!m_commandSignature.Create(l_device), "GPUパーティクル用CommandSignatureの作成に失敗しました。", false);

    m_isCreated = true;

    return true;
}

bool FWK::Graphics::GPUParticleSystem::DispatchInitialize(Renderer& a_renderer, const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress)
{
    FWK_ASSERT_RETURN_VALUE_IF(!DispatchCompute(a_renderer,
                                                Enum::PipelineStateType::GPUParticleInitialize,
                                                a_passConstantBufferAddress,
                                                Utility::DivideRoundUp(static_cast<UINT>(Constant::k_gpuParticleMaxCount), Constant::k_gpuParticleInitializeThreadCount)),
                                                "GPUパーティクルの初期化Dispatchに失敗しました。",
                                                false);

    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    // DeadListとカウンターを書き終わってから、発生が読めるようにする
    l_directCommandList.UAVResourceBarrier(m_deadListBuffer.GetREFBufferGPUResource().m_resource);
    l_directCommandList.UAVResourceBarrier(m_counterBuffer.GetREFBufferGPUResource().m_resource);

    return true;
}

FWK::Struct::CBGPUParticlePass FWK::Graphics::GPUParticleSystem::CreatePassConstantBuffer(const FrameData&    a_frameData,
                                                                                          const std::uint32_t a_emitterCount,
                                                                                          const std::uint32_t a_totalEmitCount,
                                                                                          const std::size_t   a_aliveReadListIndex,
                                                                                          const float         a_deltaTime) const
{
    const auto  l_aliveWriteListIndex  = FetchVALOtherAliveListIndex(a_aliveReadListIndex);
    const auto& l_aliveListReadBuffer  = m_aliveListBufferList[a_aliveReadListIndex];
    const auto& l_aliveListWriteBuffer = m_aliveListBufferList[l_aliveWriteListIndex];

    Struct::CBGPUParticlePass l_passConstantBuffer = {};

    l_passConstantBuffer.m_particleBufferUAVDescriptorIndex      = m_particleBuffer.GetVALUAVDescriptorIndex     ();
    l_passConstantBuffer.m_particleBufferSRVDescriptorIndex      = m_particleBuffer.GetVALSRVDescriptorIndex     ();
    l_passConstantBuffer.m_deadListBufferUAVDescriptorIndex      = m_deadListBuffer.GetVALUAVDescriptorIndex     ();
    l_passConstantBuffer.m_aliveListReadBufferUAVDescriptorIndex = l_aliveListReadBuffer.GetVALUAVDescriptorIndex();

    l_passConstantBuffer.m_aliveListWriteBufferUAVDescriptorIndex = l_aliveListWriteBuffer.GetVALUAVDescriptorIndex();
    l_passConstantBuffer.m_aliveListWriteBufferSRVDescriptorIndex = l_aliveListWriteBuffer.GetVALSRVDescriptorIndex();
    l_passConstantBuffer.m_counterBufferUAVDescriptorIndex        = m_counterBuffer.GetVALUAVDescriptorIndex       ();
    l_passConstantBuffer.m_counterBufferSRVDescriptorIndex        = m_counterBuffer.GetVALSRVDescriptorIndex       ();

    l_passConstantBuffer.m_indirectArgumentsBufferUAVDescriptorIndex = m_indirectArgumentsBuffer.GetVALUAVDescriptorIndex();
    l_passConstantBuffer.m_emitterBufferSRVDescriptorIndex           = a_frameData.m_emitterBuffer.GetVALSRVDescriptorIndex();
    l_passConstantBuffer.m_emitterCount                              = a_emitterCount;
    l_passConstantBuffer.m_totalEmitCount                            = a_totalEmitCount;

    l_passConstantBuffer.m_maxParticleCount       = static_cast<std::uint32_t>(Constant::k_gpuParticleMaxCount);
    l_passConstantBuffer.m_aliveReadCounterIndex  = Constant::k_gpuParticleAliveCounterIndexList[a_aliveReadListIndex];
    l_passConstantBuffer.m_aliveWriteCounterIndex = Constant::k_gpuParticleAliveCounterIndexList[l_aliveWriteListIndex];

    l_passConstantBuffer.m_randomSeed = m_randomSeed;
    l_passConstantBuffer.m_deltaTime  = a_deltaTime;
    l_passConstantBuffer.m_gravity    = Constant::k_gpuParticleDefaultGravity;

    return l_passConstantBuffer;
}

D3D12_GPU_VIRTUAL_ADDRESS FWK::Graphics::GPUParticleSystem::WritePassConstantBuffer(const Struct::CBGPUParticlePass& a_passConstantBuffer, const std::size_t a_slotIndex, const FrameData& a_frameData) const
{
    FWK_ASSERT_RETURN_VALUE_IF(a_slotIndex >= Constant::k_gpuParticleConstantBufferSlotCount, "定数バッファの枡の番号が範囲外のため、GPUパーティクル用定数バッファの書き込みに失敗しました。", Constant::k_invalidDynamicBufferGPUVirtualAddress);

    auto* const l_mappedData  = a_frameData.m_passConstantBuffer.FetchPTRMappedData       ();
    const auto  l_baseAddress = a_frameData.m_passConstantBuffer.FetchVALGPUVirtualAddress();

    FWK_ASSERT_RETURN_VALUE_IF(!l_mappedData, "定数バッファがMapされていないため、GPUパーティクル用定数バッファの書き込みに失敗しました。", Constant::k_invalidDynamicBufferGPUVirtualAddress);

    // 定数バッファは枡ごとに256byte(定数バッファの配置単位)ずつずらして書き込む
    const auto l_offset = a_slotIndex * Constant::k_gpuParticleConstantBufferSlotStride;

    std::memcpy(l_mappedData + l_offset, &a_passConstantBuffer, sizeof(Struct::CBGPUParticlePass));

    return l_baseAddress + l_offset;
}

bool FWK::Graphics::GPUParticleSystem::DispatchCompute(Renderer&                        a_renderer,
                                                       const Enum::PipelineStateType    a_pipelineStateType,
                                                       const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress,
                                                       const UINT                       a_threadGroupCountX) const
{
    FWK_ASSERT_RETURN_VALUE_IF(!SetupComputePipeline(a_renderer, a_pipelineStateType, a_passConstantBufferAddress), "GPUパーティクル用ComputePipelineの設定に失敗しました。", false);

    if (a_threadGroupCountX == k_emptyThreadGroupCount) { return true; }

    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    l_directCommandList.Dispatch(a_threadGroupCountX, k_singleThreadGroupCount, k_singleThreadGroupCount);

    return true;
}
bool FWK::Graphics::GPUParticleSystem::DispatchComputeIndirect(Renderer&                        a_renderer,
                                                               const Enum::PipelineStateType    a_pipelineStateType,
                                                               const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress,
                                                               const UINT64&                    a_argumentBufferOffset) const
{
    FWK_ASSERT_RETURN_VALUE_IF(!SetupComputePipeline(a_renderer, a_pipelineStateType, a_passConstantBufferAddress), "GPUパーティクル用ComputePipelineの設定に失敗しました。", false);

    const auto& l_directCommandList = a_renderer.GetREFDirectCommandList();

    // Group数はGPUが書いたBufferから読む(CPUは生きている粒子の数を知らなくてよい)
    l_directCommandList.ExecuteIndirectSingleCommand(m_commandSignature.GetREFDispatchCommandSignature(),
                                                     m_indirectArgumentsBuffer.GetREFBufferGPUResource().m_resource,
                                                     a_argumentBufferOffset);

    return true;
}
bool FWK::Graphics::GPUParticleSystem::SetupComputePipeline(Renderer& a_renderer, const Enum::PipelineStateType a_pipelineStateType, const D3D12_GPU_VIRTUAL_ADDRESS& a_passConstantBufferAddress) const
{
    const auto& l_pipelineStateWeak = a_renderer.FindVALPipelineState<ComputePipelineState>(a_pipelineStateType);
    const auto& l_pipelineState     = l_pipelineStateWeak.lock                             ();

    FWK_ASSERT_RETURN_VALUE_IF(!l_pipelineState, "GPUパーティクル用ComputePipelineStateが見つかりません。", false);

    const auto& l_rootSignature = l_pipelineState->GetREFUseRootSignature().lock();

    FWK_ASSERT_RETURN_VALUE_IF(!l_rootSignature, "GPUパーティクル用RootSignatureが見つかりません。", false);

    auto& l_directCommandList = a_renderer.GetMutableREFDirectCommandList();

    l_directCommandList.SetupComputePipeline          (l_pipelineStateWeak);
    l_directCommandList.SetupComputeConstantBufferView(a_passConstantBufferAddress, *l_rootSignature, Enum::RootParameterType::CBGPUParticlePass);

    return true;
}

void FWK::Graphics::GPUParticleSystem::TransitionForCompute(const DirectCommandList& a_directCommandList)
{
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_particleBuffer,          D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_deadListBuffer,          D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_counterBuffer,           D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_indirectArgumentsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    for (auto& l_aliveListBuffer : m_aliveListBufferList)
    {
        Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, l_aliveListBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
}
void FWK::Graphics::GPUParticleSystem::TransitionForDraw(const DirectCommandList& a_directCommandList, const std::size_t a_drawListIndex)
{
    // 粒子、描画する一覧、生存数はメッシュシェーダーがStructuredBufferとして読み、
    // Group数はExecuteIndirectが引数として読む(DeadListと読み側の一覧は描画で使わないためそのまま)
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_particleBuffer,                     D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_aliveListBufferList[a_drawListIndex], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_counterBuffer,                      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    Utility::TransitionDynamicRWStructuredBuffer(a_directCommandList, m_indirectArgumentsBuffer,            D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
}

void FWK::Graphics::GPUParticleSystem::UAVBarrierEmitWrittenBuffers(const DirectCommandList& a_directCommandList, const std::size_t a_aliveReadListIndex) const
{
    // UAVBarrierは「このBufferへの先行する書き込みが終わってから、後続の読み書きを始める」ことを保証する
    a_directCommandList.UAVResourceBarrier(m_counterBuffer.GetREFBufferGPUResource                            ().m_resource);
    a_directCommandList.UAVResourceBarrier(m_deadListBuffer.GetREFBufferGPUResource                           ().m_resource);
    a_directCommandList.UAVResourceBarrier(m_particleBuffer.GetREFBufferGPUResource                           ().m_resource);
    a_directCommandList.UAVResourceBarrier(m_aliveListBufferList[a_aliveReadListIndex].GetREFBufferGPUResource().m_resource);
}

std::size_t FWK::Graphics::GPUParticleSystem::FetchVALOtherAliveListIndex(const std::size_t a_aliveListIndex) const
{
    // 一覧は2つ(0と1)なので、1から引くともう一方になる
    return k_aliveListIndexSum - a_aliveListIndex;
}