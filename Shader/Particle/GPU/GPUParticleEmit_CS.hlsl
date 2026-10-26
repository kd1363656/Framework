#include "GPUParticle.hlsli"

// GPUパーティクルの発生用ComputeShader
//
// 1つのThreadが今フレームに出す粒子1つを担当する
// 1. 自分がどの発生源(エミッター)の粒子かを二分探索で調べる
// 2. DeadList(空きスロットの山)から空きスロットを1つ取る(空きが無ければ出さない)
// 3. 発生源の設定と乱数から粒子の初期値を作り、スロットへ書く
// 4. 読み側のAliveListへスロット番号を積む(今フレームのSimulateが最初の1歩を動かす)

// 寿命が0になると「経過時間 ÷ 寿命」が計算できなくなるため、これより短い寿命は使わない
static const float k_minLifetime = 0.0001F;

// 1つの発生源が出す粒子を見分けるために、スロット番号と出す順番を混ぜるときの係数
static const uint k_randomStateSlotMultiplier = 0x9E3779B9U;

// 今フレームに出す全粒子を1列に並べた番号(a_emitIndex)から、どの発生源の粒子かを探す
// 発生源はemitOffsetが小さい順に並んでいるため、二分探索で「emitOffset <= a_emitIndex」を満たす最後の発生源を探す
// (発生源が1024個あっても10回ほどの比較で見つかる)
uint FindGPUParticleEmitterIndex(const StructuredBuffer<GPUParticleEmitter> a_emitterBuffer, const uint a_emitIndex)
{
    // [l_low, l_high)の範囲を半分ずつ狭めていく
    uint l_low  = k_gpuParticleInitialCounterValue;
    uint l_high = g_emitterCount;

    while (l_high - l_low > k_gpuParticleCountToLastIndexOffset)
    {
        const uint l_middle = (l_low + l_high) / 2U;

        if (a_emitterBuffer[l_middle].emitOffset <= a_emitIndex) { l_low  = l_middle; }
        else                                                     { l_high = l_middle; }
    }

    return l_low;
}

[numthreads(k_gpuParticleEmitThreadCountX, k_gpuParticleSingleThreadCountY, k_gpuParticleSingleThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    const uint l_emitIndex = a_dispatchThreadID.x;

    // 最後のGroupには担当の無い余分なスレッドが含まれる
    if (l_emitIndex >= g_totalEmitCount) { return; }

    StructuredBuffer<GPUParticleEmitter> l_emitterBuffer = ResourceDescriptorHeap[g_emitterBufferSRVDescriptorIndex];
    RWStructuredBuffer<uint>             l_counterBuffer = ResourceDescriptorHeap[g_counterBufferUAVDescriptorIndex];

    const GPUParticleEmitter l_emitter = l_emitterBuffer[FindGPUParticleEmitterIndex(l_emitterBuffer, l_emitIndex)];

    // 空きスロットを1つ取る
    // InterlockedAddは複数のThreadが同時に足しても正しく数えられる加算で、第三引数に「加算前の値」が入る
    // 空きの数を1減らし、減らす前の数が0以下なら空きが無いので、減らした分を戻して出さない
    uint l_deadCountBefore = k_gpuParticleInitialCounterValue;

    InterlockedAdd(l_counterBuffer[k_gpuParticleDeadCounterIndex], k_gpuParticleCounterDecrementValue, l_deadCountBefore);

    if ((int)l_deadCountBefore <= (int)k_gpuParticleInitialCounterValue)
    {
        InterlockedAdd(l_counterBuffer[k_gpuParticleDeadCounterIndex], k_gpuParticleCounterIncrementValue);

        return;
    }

    RWStructuredBuffer<uint> l_deadListBuffer = ResourceDescriptorHeap[g_deadListBufferUAVDescriptorIndex];

    // 山の一番上(空きの数 - 1番目)に積まれているスロット番号を取る
    const uint l_slotIndex = l_deadListBuffer[l_deadCountBefore - k_gpuParticleCountToLastIndexOffset];

    // 乱数の種 : スロット番号、出す順番、フレームごとの種を混ぜる(同じ発生源の粒子が同じ値にならないようにする)
    uint l_randomState = HashGPUParticle(l_slotIndex * k_randomStateSlotMultiplier + l_emitIndex) ^ g_randomSeed;

    const float3 l_positionRandom = RandomSignedGPUParticle3(l_randomState);
    const float3 l_velocityRandom = RandomSignedGPUParticle3(l_randomState);
    const float  l_lifetimeRandom = RandomGPUParticle       (l_randomState);
    const float  l_rotationRandom = RandomGPUParticle       (l_randomState);
    const float  l_angularRandom  = RandomGPUParticle       (l_randomState);
    const float  l_speedRandom    = RandomGPUParticle       (l_randomState);

    GPUParticle l_particle = (GPUParticle)k_gpuParticleZeroInitializeValue;

    // 位置 : 発生源の位置を中心に、positionRandomRangeの箱の中からランダムに選ぶ
    l_particle.position = l_emitter.position + l_positionRandom * l_emitter.positionRandomRange;
    l_particle.age      = k_gpuParticleInitialCounterValue;

    // 速度 : 基本の速度に、ランダムな向き × ランダムな速さ(最大velocityRandomRange)を足す
    // (方向のベクトルの長さがほぼ0のときにnormalizeすると値が壊れるため、normalizeせずに-1～1の箱の中の点をそのまま使う)
    l_particle.velocity = l_emitter.velocity + l_velocityRandom * (l_emitter.velocityRandomRange * l_speedRandom);
    l_particle.lifetime = max(lerp(l_emitter.lifetimeMIN, l_emitter.lifetimeMAX, l_lifetimeRandom), k_minLifetime);

    l_particle.colorBegin = PackGPUParticleColor(l_emitter.colorBegin);
    l_particle.colorEnd   = PackGPUParticleColor(l_emitter.colorEnd);

    l_particle.sizeBeginEnd     = PackGPUParticleHalf2(float2(l_emitter.sizeBegin, l_emitter.sizeEnd));
    l_particle.rotation         = (l_rotationRandom * k_gpuParticleRandomSignedScale - k_gpuParticleRandomSignedOffset) * l_emitter.rotationRandomRange;
    l_particle.angularVelocity  = (l_angularRandom  * k_gpuParticleRandomSignedScale - k_gpuParticleRandomSignedOffset) * l_emitter.angularVelocityRandomRange;
    l_particle.dragGravityScale = PackGPUParticleHalf2(float2(l_emitter.drag, l_emitter.gravityScale));

    RWStructuredBuffer<GPUParticle> l_particleBuffer = ResourceDescriptorHeap[g_particleBufferUAVDescriptorIndex];

    l_particleBuffer[l_slotIndex] = l_particle;

    // 読み側のAliveListの末尾へスロット番号を積む
    // (生きている粒子の数は最大でもスロット数なので、Bufferの外へ書くことは無い)
    RWStructuredBuffer<uint> l_aliveListReadBuffer = ResourceDescriptorHeap[g_aliveListReadBufferUAVDescriptorIndex];

    uint l_aliveIndex = k_gpuParticleInitialCounterValue;

    InterlockedAdd(l_counterBuffer[g_aliveReadCounterIndex], k_gpuParticleCounterIncrementValue, l_aliveIndex);

    l_aliveListReadBuffer[l_aliveIndex] = l_slotIndex;
}