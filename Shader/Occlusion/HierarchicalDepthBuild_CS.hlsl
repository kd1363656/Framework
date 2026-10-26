// Hi-Z(階層深度バッファ)を作るComputeShader
//
// Hi-Zは、深度バッファを半分、さらに半分…と縮小したテクスチャの集まり(ミップマップ)。
// 縮小するときに「範囲内で一番奥(値が大きい)の深度」を残すことで、
// 縮小後の1ピクセルは「その範囲に描かれている物の中で一番奥の深さ」を表す。
//
// 物体の範囲を覆うミップの値より、物体の一番手前の深さの方が奥にあれば、
// その物体は範囲内の何かに完全に隠されていると判断できる(オクルージョンカリング)
//
// このShaderは1回の実行でミップ1段分を作る
// 1段目(ミップ0)は深度バッファから、2段目以降は1つ前のミップから作る

cbuffer CBHierarchicalDepthBuildPass : register(b0)
{
    uint  g_sourceDescriptorIndex;
    uint  g_isSourceDepthTexture;
    uint2 g_sourceSize;

    uint  g_destinationUAVDescriptorIndex;
    uint  g_padding;
    uint2 g_destinationSize;
};

static const uint k_hierarchicalDepthBuildThreadCountX = 8U;
static const uint k_hierarchicalDepthBuildThreadCountY = 8U;
static const uint k_hierarchicalDepthBuildThreadCountZ = 1U;

static const uint k_sourceMIPLevel = 0U;

static const uint k_trueValue = 1U;

// 深度の最小値(一番手前)
static const float k_nearestDepth = 0.0F;

// 隣のピクセル(1つ先)を指すときに足す値
static const uint k_nextPixelOffset = 1U;

// 割り算を切り上げにするときに、割る数から引く値(「割られる数 + 割る数 - 1」を割ると切り上げになる)
static const uint k_ceilDivideOffset = 1U;

// 元テクスチャの1ピクセル分の深度を読む
float LoadSourceDepth(const uint2 a_position)
{
    // ミップ0の作成時は深度バッファ(SRV)、それ以外は1つ前のミップ(UAV)から読む
    if (g_isSourceDepthTexture == k_trueValue)
    {
        Texture2D<float> l_depthTexture = ResourceDescriptorHeap[g_sourceDescriptorIndex];

        return l_depthTexture.Load(int3(a_position, k_sourceMIPLevel));
    }

    RWTexture2D<float> l_sourceTexture = ResourceDescriptorHeap[g_sourceDescriptorIndex];

    return l_sourceTexture[a_position];
}

[numthreads(k_hierarchicalDepthBuildThreadCountX, k_hierarchicalDepthBuildThreadCountY, k_hierarchicalDepthBuildThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    const uint2 l_destinationPosition = a_dispatchThreadID.xy;

    if (any(l_destinationPosition >= g_destinationSize)) { return; }

    // 書き込み先の1ピクセルが、元テクスチャのどの範囲に当たるかを求める
    // (2段目以降はちょうど2×2、1段目は大きさが2の累乗に切り捨てられているため最大3×3になる)
    // 範囲の始まり = 書き込み先の位置 × 元の大きさ ÷ 書き込み先の大きさ(切り捨て)
    // 範囲の終わり = 次の書き込み先の位置 × 元の大きさ ÷ 書き込み先の大きさ(切り上げ、元の大きさを超えない)
    const uint2 l_nextDestinationPosition = l_destinationPosition + k_nextPixelOffset;
    const uint2 l_sourceBegin             = l_destinationPosition * g_sourceSize / g_destinationSize;
    const uint2 l_sourceEnd               = min((l_nextDestinationPosition * g_sourceSize + g_destinationSize - k_ceilDivideOffset) / g_destinationSize, g_sourceSize);

    // 範囲内で一番奥(最大)の深度を求める
    float l_maxDepth = k_nearestDepth;

    for (uint l_y = l_sourceBegin.y; l_y < l_sourceEnd.y; ++l_y)
    {
        for (uint l_x = l_sourceBegin.x; l_x < l_sourceEnd.x; ++l_x)
        {
            l_maxDepth = max(l_maxDepth, LoadSourceDepth(uint2(l_x, l_y)));
        }
    }

    RWTexture2D<float> l_destinationTexture = ResourceDescriptorHeap[g_destinationUAVDescriptorIndex];

    l_destinationTexture[l_destinationPosition] = l_maxDepth;
}