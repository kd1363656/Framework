#include "GPUParticle.hlsli"

// GPUパーティクルの初期化用ComputeShader(Bufferを作った直後に1回だけ実行する)
//
// 全てのスロットを「空き」としてDeadListへ積み、カウンターを初期値にする
// DeadListは「空きスロットの番号を積んだ山」で、カウンター(空きの数)が山の高さを表す
// Emitは山の一番上から番号を取り、Simulateは寿命が来た粒子の番号を山の上へ戻す

[numthreads(k_gpuParticleInitializeThreadCountX, k_gpuParticleSingleThreadCountY, k_gpuParticleSingleThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    const uint l_slotIndex = a_dispatchThreadID.x;

    // C++側ではスロット数をGroupの大きさで切り上げてDispatchするため、余分なスレッドはここで終了する
    if (l_slotIndex >= g_maxParticleCount) { return; }

    RWStructuredBuffer<uint> l_deadListBuffer = ResourceDescriptorHeap[g_deadListBufferUAVDescriptorIndex];

    // 最初は全スロットが空きなので、「スロット番号 = 山の中の位置」としてそのまま積む
    l_deadListBuffer[l_slotIndex] = l_slotIndex;

    // カウンターはBuffer全体で1組しか無いため、先頭のスレッドだけが書く
    if (l_slotIndex != k_gpuParticleLeaderThreadIndex) { return; }

    RWStructuredBuffer<uint> l_counterBuffer = ResourceDescriptorHeap[g_counterBufferUAVDescriptorIndex];

    l_counterBuffer[k_gpuParticleDeadCounterIndex] = g_maxParticleCount;
    l_counterBuffer[g_aliveReadCounterIndex]       = k_gpuParticleInitialCounterValue;
    l_counterBuffer[g_aliveWriteCounterIndex]      = k_gpuParticleInitialCounterValue;
}