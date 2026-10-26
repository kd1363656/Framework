#ifndef GPU_PARTICLE_HLSLI
#define GPU_PARTICLE_HLSLI

// GPUパーティクルの共通定義
// ※ C++側(Definition/Struct/Graphics/Buffer/Structured/GPUParticleStruct.h、
//    Definition/Struct/Graphics/Buffer/Constant/CBGPUParticlePassStruct.h)と必ず同じ並びにする
//
// 仕組み(粒子の生成・移動・描画が全てGPU上で完結し、CPUは「今フレーム出す数と設定」を送るだけ)
// 1. Initialize : 全スロット番号をDeadList(空きスロットの山)へ積む(最初の1回だけ)
// 2. Emit       : DeadListから空きスロットを1つ取り、初期値を書き、AliveList(読み側)へ番号を積む
// 3. Simulate   : AliveList(読み側)の粒子を1Threadずつ動かし、生きていればAliveList(書き側)へ、寿命が来たらDeadListへ番号を戻す
// 4. Draw       : AliveList(書き側)の順にメッシュシェーダーが板(ビルボード)を作る
// 生きている粒子の数はGPUのカウンターだけが知っているため、
// SimulateのDispatch数と描画のDispatchMesh数はIndirectArgumentsBuildが書き、ExecuteIndirectでGPUが自分で決める

// 粒子1つ分(64byte。1Threadが1粒子を読み書きするため、小さいほど速い)
// colorBegin / colorEnd : RGBAをそれぞれ16bitの浮動小数(half)にして2つのuintへ詰めた色(f32tof16 / f16tof32で変換する)
// sizeBeginEnd          : x = 生まれたときの大きさ、y = 寿命が来たときの大きさ(halfを2つ詰めた物)
// dragGravityScale      : x = 空気抵抗、y = 重力の効き具合(halfを2つ詰めた物)
struct GPUParticle
{
    float3 position;
    float  age;

    float3 velocity;
    float  lifetime;

    uint2 colorBegin;
    uint2 colorEnd;

    uint  sizeBeginEnd;
    float rotation;
    float angularVelocity;
    uint  dragGravityScale;
};

// 発生源(エミッター)1つ分。CPUが毎フレーム「このフレームに出す粒子の設定」として送る
// emitOffset : 今フレームに出す全粒子を1列に並べたとき、この発生源の粒子が何番目から始まるか
// emitCount  : この発生源が今フレームに出す数
struct GPUParticleEmitter
{
    float3 position;
    uint   emitOffset;

    float3 positionRandomRange;
    uint   emitCount;

    float3 velocity;
    float  velocityRandomRange;

    float4 colorBegin;
    float4 colorEnd;

    float lifetimeMIN;
    float lifetimeMAX;
    float sizeBegin;
    float sizeEnd;

    float rotationRandomRange;
    float angularVelocityRandomRange;
    float drag;
    float gravityScale;
};

// ExecuteIndirectが読む引数(Simulate用のDispatchのGroup数と、描画用のDispatchMeshのGroup数)
// C++側のStruct::GPUParticleIndirectArgumentsと同じ並び(それぞれ16byteの区切りに置く)
struct GPUParticleIndirectArguments
{
    uint3 simulateThreadGroupCount;
    uint  padding0;

    uint3 drawThreadGroupCount;
    uint  padding1;
};

// 1フレーム分の設定(全てのComputeShaderとメッシュシェーダーが同じ物を読む)
// g_aliveReadCounterIndex / g_aliveWriteCounterIndex : CounterBufferの何番目が読み側 / 書き側の生存数か(毎フレーム入れ替わる)
// g_indirectArgumentsBuildTarget                      : IndirectArgumentsBuild_CSがどちらのGroup数を書くか(C++側のEnum::GPUParticleIndirectArgumentsBuildTarget)
// g_randomSeed                                        : フレームごとに変わる乱数の種(同じスロットでも毎フレーム違う乱数になる)
cbuffer CBGPUParticlePass : register(b1)
{
    uint g_particleBufferUAVDescriptorIndex;
    uint g_particleBufferSRVDescriptorIndex;
    uint g_deadListBufferUAVDescriptorIndex;
    uint g_aliveListReadBufferUAVDescriptorIndex;

    uint g_aliveListWriteBufferUAVDescriptorIndex;
    uint g_aliveListWriteBufferSRVDescriptorIndex;
    uint g_counterBufferUAVDescriptorIndex;
    uint g_counterBufferSRVDescriptorIndex;

    uint g_indirectArgumentsBufferUAVDescriptorIndex;
    uint g_emitterBufferSRVDescriptorIndex;
    uint g_emitterCount;
    uint g_totalEmitCount;

    uint g_maxParticleCount;
    uint g_aliveReadCounterIndex;
    uint g_aliveWriteCounterIndex;
    uint g_indirectArgumentsBuildTarget;

    uint  g_randomSeed;
    float g_deltaTime;
    uint2 g_gpuParticlePassPadding;

    float3 g_gravity;
    float  g_gpuParticlePassPadding2;
};

// CounterBufferの要素番号(C++側のConstant::k_gpuParticleDeadCounterIndexと必ず同じ値にする)
// 生存数の2つ(読み側、書き側)はg_aliveReadCounterIndex / g_aliveWriteCounterIndexで受け取る
static const uint k_gpuParticleDeadCounterIndex = 0U;

// IndirectArgumentsBuild_CSが書く対象(C++側のEnum::GPUParticleIndirectArgumentsBuildTargetと同じ値)
static const uint k_gpuParticleIndirectArgumentsBuildTargetSimulate = 0U;
static const uint k_gpuParticleIndirectArgumentsBuildTargetDraw     = 1U;

// 1Groupあたりのスレッド数(C++側のConstant::k_gpuParticleXXXThreadCountと必ず同じ値にする)
static const uint k_gpuParticleInitializeThreadCountX = 256U;
static const uint k_gpuParticleEmitThreadCountX       = 256U;
static const uint k_gpuParticleSimulateThreadCountX   = 256U;

// メッシュシェーダー1Groupが担当する粒子数(C++側のConstant::k_gpuParticleMeshShaderThreadCountと必ず同じ値にする)
// 1粒子 = 4頂点2三角形なので、64粒子で256頂点128三角形(メッシュシェーダー1Groupの出力上限)になる
static const uint k_gpuParticleMeshShaderThreadCountX        = 64U;
static const uint k_gpuParticleVertexCountPerParticle        = 4U;
static const uint k_gpuParticleTriangleCountPerParticle      = 2U;
static const uint k_gpuParticleMaxMeshShaderVertexCount      = k_gpuParticleMeshShaderThreadCountX * k_gpuParticleVertexCountPerParticle;
static const uint k_gpuParticleMaxMeshShaderPrimitiveCount   = k_gpuParticleMeshShaderThreadCountX * k_gpuParticleTriangleCountPerParticle;

static const uint k_gpuParticleSingleThreadCountX = 1U;
static const uint k_gpuParticleSingleThreadCountY = 1U;
static const uint k_gpuParticleSingleThreadCountZ = 1U;

static const uint k_gpuParticleSingleThreadGroupCountY = 1U;
static const uint k_gpuParticleSingleThreadGroupCountZ = 1U;

// IndirectArgumentsは1要素だけのBufferなので、常に0番目を読み書きする
static const uint k_gpuParticleIndirectArgumentsElementIndex = 0U;

// Group全体で1回だけ行う処理を担当するスレッドの番号
static const uint k_gpuParticleLeaderThreadIndex = 0U;

// InterlockedAddでカウンターを1つ進める / 戻すときに足す値(uintの0xFFFFFFFFは、int型で見ると-1)
static const uint k_gpuParticleCounterIncrementValue = 1U;
static const uint k_gpuParticleCounterDecrementValue = 0xFFFFFFFFU;

static const uint k_gpuParticleInitialCounterValue = 0U;

// 「数」から「最後の番号」へ直すときに引く値(例 : 空きの数 - 1 = 一番上の空きスロットの番号)
static const uint k_gpuParticleCountToLastIndexOffset = 1U;

// 構造体を全て0で初期化するときに使う値
static const uint k_gpuParticleZeroInitializeValue = 0U;

// halfを2つ1つのuintへ詰めるときのビット数とマスク
static const uint k_gpuParticleHalfBitCount = 16U;
static const uint k_gpuParticleHalfMask     = 0xFFFFU;

// PCGハッシュ(乱数生成)で使う定数
static const uint k_gpuParticlePCGMultiplier       = 747796405U;
static const uint k_gpuParticlePCGIncrement        = 2891336453U;
static const uint k_gpuParticlePCGShiftA           = 28U;
static const uint k_gpuParticlePCGShiftB           = 4U;
static const uint k_gpuParticlePCGOutputMultiplier = 277803737U;
static const uint k_gpuParticlePCGOutputShift      = 22U;

// 32bitの乱数の上位24bitを使って0以上1未満の浮動小数を作る(1 / 2^24)
static const uint  k_gpuParticleRandomMantissaShift = 8U;
static const float k_gpuParticleRandomMantissaScale = 1.0F / 16777216.0F;

// 0～1の乱数を-1～1へ広げるときの係数
static const float k_gpuParticleRandomSignedScale  = 2.0F;
static const float k_gpuParticleRandomSignedOffset = 1.0F;

// 整数を混ぜ合わせて別の整数にする(PCGハッシュ)
// GPUには乱数を作る関数が無いため、「同じ入力なら必ず同じ出力になるが、入力が1違うだけで全く違う出力になる」計算を乱数の代わりに使う
uint HashGPUParticle(const uint a_value)
{
    const uint l_state = a_value * k_gpuParticlePCGMultiplier + k_gpuParticlePCGIncrement;
    const uint l_word  = ((l_state >> ((l_state >> k_gpuParticlePCGShiftA) + k_gpuParticlePCGShiftB)) ^ l_state) * k_gpuParticlePCGOutputMultiplier;

    return (l_word >> k_gpuParticlePCGOutputShift) ^ l_word;
}

// 0以上1未満の乱数を1つ取り出す(取り出すたびにa_stateが次の値へ進む)
float RandomGPUParticle(inout uint a_state)
{
    a_state = HashGPUParticle(a_state);

    return (float)(a_state >> k_gpuParticleRandomMantissaShift) * k_gpuParticleRandomMantissaScale;
}

// -1以上1未満の乱数を3つ取り出す
float3 RandomSignedGPUParticle3(inout uint a_state)
{
    const float3 l_random = float3(RandomGPUParticle(a_state), RandomGPUParticle(a_state), RandomGPUParticle(a_state));

    return l_random * k_gpuParticleRandomSignedScale - k_gpuParticleRandomSignedOffset;
}

// 2つのfloatを16bitずつにしてuint1つへ詰める(x = 下位16bit、y = 上位16bit)
uint PackGPUParticleHalf2(const float2 a_value)
{
    return f32tof16(a_value.x) | (f32tof16(a_value.y) << k_gpuParticleHalfBitCount);
}

// PackGPUParticleHalf2で詰めた物を2つのfloatへ戻す
float2 UnpackGPUParticleHalf2(const uint a_packed)
{
    return float2(f16tof32(a_packed & k_gpuParticleHalfMask), f16tof32(a_packed >> k_gpuParticleHalfBitCount));
}

// RGBAの色を16bitずつにして2つのuintへ詰める(float4のままだと16byte、詰めると8byte)
uint2 PackGPUParticleColor(const float4 a_color)
{
    return uint2(PackGPUParticleHalf2(a_color.rg), PackGPUParticleHalf2(a_color.ba));
}

// PackGPUParticleColorで詰めた物を色へ戻す
float4 UnpackGPUParticleColor(const uint2 a_packed)
{
    return float4(UnpackGPUParticleHalf2(a_packed.x), UnpackGPUParticleHalf2(a_packed.y));
}

// 「数 ÷ 1Groupあたりの数」を切り上げてGroup数にする(余りがあるときだけ1つ増える)
uint CalculateGPUParticleThreadGroupCount(const uint a_threadCount, const uint a_threadCountPerGroup)
{
    return (a_threadCount + a_threadCountPerGroup - k_gpuParticleCountToLastIndexOffset) / a_threadCountPerGroup;
}

#endif // GPU_PARTICLE_HLSLI