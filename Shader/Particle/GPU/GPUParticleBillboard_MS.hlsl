#include "GPUParticleBillboard.hlsli"

// GPUパーティクルをビルボード(常にカメラの方を向く板)として描くメッシュシェーダー
//
// 頂点バッファを使わず、1Groupが書き側のAliveListから64粒子を読み、
// 粒子ごとに4頂点2三角形の板をその場で作る(1Groupの出力上限は256頂点128三角形)
// Group数はIndirectArgumentsBuild_CSが書いた「生存数 ÷ 64(切り上げ)」で、ExecuteIndirectが起動する

// 板の4つの角(中心からの向き。-1～1)と、その角のuv
static const float2 k_cornerOffsetList[k_gpuParticleVertexCountPerParticle] =
{
    float2(-1.0F, -1.0F),
    float2( 1.0F, -1.0F),
    float2( 1.0F,  1.0F),
    float2(-1.0F,  1.0F),
};

static const float2 k_cornerUVList[k_gpuParticleVertexCountPerParticle] =
{
    float2(0.0F, 1.0F),
    float2(1.0F, 1.0F),
    float2(1.0F, 0.0F),
    float2(0.0F, 0.0F),
};

// 2つの三角形が使う角の番号(0-1-2と0-2-3)
static const uint3 k_triangleCornerIndexList[k_gpuParticleTriangleCountPerParticle] =
{
    uint3(0U, 1U, 2U),
    uint3(0U, 2U, 3U),
};

// 「大きさ」は板の1辺の長さなので、中心からの距離は半分
static const float k_halfSizeScale = 0.5F;

static const float k_positionElementW = 1.0F;

[outputtopology("triangle")]
[numthreads(k_gpuParticleMeshShaderThreadCountX, k_gpuParticleSingleThreadCountY, k_gpuParticleSingleThreadCountZ)]
void main(const uint3                a_groupID          : SV_GroupID,
          const uint                 a_groupThreadIndex : SV_GroupIndex,
          out vertices GPUParticleMSOutput a_vertexList   [k_gpuParticleMaxMeshShaderVertexCount],
          out indices  uint3               a_primitiveList[k_gpuParticleMaxMeshShaderPrimitiveCount])
{
    StructuredBuffer<uint> l_counterBuffer = ResourceDescriptorHeap[g_counterBufferSRVDescriptorIndex];

    // このGroupが担当する粒子の範囲(最後のGroupは64個に満たないことがある)
    const uint l_aliveCount         = l_counterBuffer[g_aliveWriteCounterIndex];
    const uint l_groupFirstIndex    = a_groupID.x * k_gpuParticleMeshShaderThreadCountX;
    const uint l_groupParticleCount = l_groupFirstIndex < l_aliveCount ? min(l_aliveCount - l_groupFirstIndex, k_gpuParticleMeshShaderThreadCountX) : k_gpuParticleInitialCounterValue;

    // 出力する頂点数と三角形数は、Group内の全スレッドが同じ値で1回だけ宣言する
    SetMeshOutputCounts(l_groupParticleCount * k_gpuParticleVertexCountPerParticle, l_groupParticleCount * k_gpuParticleTriangleCountPerParticle);

    if (a_groupThreadIndex >= l_groupParticleCount) { return; }

    StructuredBuffer<uint>        l_aliveListBuffer = ResourceDescriptorHeap[g_aliveListWriteBufferSRVDescriptorIndex];
    StructuredBuffer<GPUParticle> l_particleBuffer  = ResourceDescriptorHeap[g_particleBufferSRVDescriptorIndex];

    const uint        l_slotIndex = l_aliveListBuffer[l_groupFirstIndex + a_groupThreadIndex];
    const GPUParticle l_particle  = l_particleBuffer [l_slotIndex];

    // 寿命のどこまで進んだか(0 = 生まれた直後、1 = 寿命)で、大きさと色を補間する
    const float  l_lifeRatio    = saturate(l_particle.age / l_particle.lifetime);
    const float2 l_sizeBeginEnd = UnpackGPUParticleHalf2(l_particle.sizeBeginEnd);
    const float  l_halfSize     = lerp(l_sizeBeginEnd.x, l_sizeBeginEnd.y, l_lifeRatio) * k_halfSizeScale;
    const float4 l_color        = lerp(UnpackGPUParticleColor(l_particle.colorBegin), UnpackGPUParticleColor(l_particle.colorEnd), l_lifeRatio);

    // カメラの右向きと上向き(ワールド座標)
    // ビュー行列はワールド → カメラの変換で、回転部分の逆行列は転置なので、列を取り出すとカメラの軸になる
    const float3 l_cameraRight = float3(g_viewMatrix._11, g_viewMatrix._21, g_viewMatrix._31);
    const float3 l_cameraUp    = float3(g_viewMatrix._12, g_viewMatrix._22, g_viewMatrix._32);

    // 板をカメラの視線を軸に回転させる(回転後の右向きと上向きを求めておけば、角の計算は足し算だけで済む)
    const float  l_cosine = cos(l_particle.rotation);
    const float  l_sine   = sin(l_particle.rotation);
    const float3 l_axisX  = (l_cameraRight * l_cosine + l_cameraUp * l_sine) * l_halfSize;
    const float3 l_axisY  = (l_cameraUp * l_cosine - l_cameraRight * l_sine) * l_halfSize;

    const uint l_vertexBaseIndex   = a_groupThreadIndex * k_gpuParticleVertexCountPerParticle;
    const uint l_triangleBaseIndex = a_groupThreadIndex * k_gpuParticleTriangleCountPerParticle;

    [unroll]
    for (uint l_cornerIndex = 0U; l_cornerIndex < k_gpuParticleVertexCountPerParticle; ++l_cornerIndex)
    {
        const float2 l_cornerOffset  = k_cornerOffsetList[l_cornerIndex];
        const float3 l_worldPosition = l_particle.position + l_axisX * l_cornerOffset.x + l_axisY * l_cornerOffset.y;

        GPUParticleMSOutput l_output = (GPUParticleMSOutput)k_gpuParticleZeroInitializeValue;

        l_output.position = mul(float4(l_worldPosition, k_positionElementW), g_viewProjectionMatrix);
        l_output.uv       = k_cornerUVList[l_cornerIndex];
        l_output.color    = l_color;

        a_vertexList[l_vertexBaseIndex + l_cornerIndex] = l_output;
    }

    [unroll]
    for (uint l_triangleIndex = 0U; l_triangleIndex < k_gpuParticleTriangleCountPerParticle; ++l_triangleIndex)
    {
        // 角の番号は粒子の中での番号なので、このGroupの出力頂点の中での番号へずらす
        a_primitiveList[l_triangleBaseIndex + l_triangleIndex] = k_triangleCornerIndexList[l_triangleIndex] + l_vertexBaseIndex;
    }
}