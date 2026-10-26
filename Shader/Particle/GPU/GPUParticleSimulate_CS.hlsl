#include "GPUParticle.hlsli"

// GPUパーティクルの移動用ComputeShader
//
// 1つのThreadが読み側のAliveListにある粒子1つを担当する
// 1. 経過時間を進め、寿命が来ていればスロット番号をDeadListへ戻して終わる
// 2. 重力と空気抵抗で速度を変え、速度で位置を、角速度で回転を進める
// 3. 書き側のAliveListへスロット番号を積む(次のフレームはこの一覧が読み側になる)
//
// 生きている粒子だけを一覧にして詰め直す(コンパクション)ため、
// 死んだ粒子のスロットは次のSimulateで一切読まれず、描画もされない

// 空気抵抗で速度を減らすときに使う値
// 速度 × 1 / (1 + 抵抗 × 経過時間) にする(経過時間が大きくても速度の向きが反転しない)
static const float k_dragDenominatorBase = 1.0F;

[numthreads(k_gpuParticleSimulateThreadCountX, k_gpuParticleSingleThreadCountY, k_gpuParticleSingleThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    RWStructuredBuffer<uint> l_counterBuffer = ResourceDescriptorHeap[g_counterBufferUAVDescriptorIndex];

    const uint l_aliveIndex = a_dispatchThreadID.x;
    const uint l_aliveCount = l_counterBuffer[g_aliveReadCounterIndex];

    // Group数は切り上げで決まっているため、最後のGroupには担当の無い余分なスレッドが含まれる
    if (l_aliveIndex >= l_aliveCount) { return; }

    RWStructuredBuffer<uint>        l_aliveListReadBuffer = ResourceDescriptorHeap[g_aliveListReadBufferUAVDescriptorIndex];
    RWStructuredBuffer<GPUParticle> l_particleBuffer      = ResourceDescriptorHeap[g_particleBufferUAVDescriptorIndex];

    const uint l_slotIndex = l_aliveListReadBuffer[l_aliveIndex];

    GPUParticle l_particle = l_particleBuffer[l_slotIndex];

    l_particle.age += g_deltaTime;

    // 寿命が来たら、スロット番号をDeadList(空きスロットの山)の上へ戻す
    if (l_particle.age >= l_particle.lifetime)
    {
        RWStructuredBuffer<uint> l_deadListBuffer = ResourceDescriptorHeap[g_deadListBufferUAVDescriptorIndex];

        uint l_deadIndex = k_gpuParticleInitialCounterValue;

        InterlockedAdd(l_counterBuffer[k_gpuParticleDeadCounterIndex], k_gpuParticleCounterIncrementValue, l_deadIndex);

        l_deadListBuffer[l_deadIndex] = l_slotIndex;

        return;
    }

    // x = 空気抵抗、y = 重力の効き具合
    const float2 l_dragGravityScale = UnpackGPUParticleHalf2(l_particle.dragGravityScale);

    l_particle.velocity += g_gravity * (l_dragGravityScale.y * g_deltaTime);
    l_particle.velocity *= rcp(k_dragDenominatorBase + l_dragGravityScale.x * g_deltaTime);
    l_particle.position += l_particle.velocity * g_deltaTime;
    l_particle.rotation += l_particle.angularVelocity * g_deltaTime;

    l_particleBuffer[l_slotIndex] = l_particle;

    // 書き側のAliveListの末尾へスロット番号を積む
    RWStructuredBuffer<uint> l_aliveListWriteBuffer = ResourceDescriptorHeap[g_aliveListWriteBufferUAVDescriptorIndex];

    uint l_aliveWriteIndex = k_gpuParticleInitialCounterValue;

    InterlockedAdd(l_counterBuffer[g_aliveWriteCounterIndex], k_gpuParticleCounterIncrementValue, l_aliveWriteIndex);

    l_aliveListWriteBuffer[l_aliveWriteIndex] = l_slotIndex;
}