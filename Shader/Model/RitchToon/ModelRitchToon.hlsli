#ifndef MODEL_RITCH_TOON_HLSLI
#define MODEL_RITCH_TOON_HLSLI
#include "../Standard/ModelStandard.hlsli"
#include "../../Light/LightPass.hlsli"
#include "../../Material/RitchToon/RitchToonMaterial.hlsli"

// トゥーン調では、ライトが強くても色がテクスチャより明るくなりすぎないように、
// ライトの色(ディレクショナルライト + 環境光)をこの値までに抑える
static const float k_ritchToonMaxLightColor = 1.0F;

// 輪郭線マスクは、MeshShaderの中で頂点ごとに読む
// RootSignatureはSampler(テクスチャの読み方の設定)をPixelShaderにしか渡していないため、
// MeshShaderではSamplerを使わずに、テクスチャの1ピクセル(テクセル)を番号で直接読む(Load)
// 一番細かいMip(0番)から読む
static const uint k_ritchToonOutlineMaskMipLevel = 0U;

// ピクセルの番号は0 ~ (幅 - 1)なので、最後のピクセルの番号は「幅 - 1」になる
static const uint2 k_ritchToonLastTexelOffset = uint2(1U, 1U);

static const uint k_ritchToonInitialTextureDimension = 0U;

// Octahedral符号化で詰めた向きを戻すための値
// ※ C++側(ModelOutlineNormalBuilder::EncodeOctahedralNormal)と必ず同じ詰め方にする
// x : 下位16bit、y : 上位16bit、それぞれ0 ~ 65535で-1.0F ~ 1.0Fを表す
static const uint  k_ritchToonOctahedralLowMask  = 0xFFFFU;
static const uint  k_ritchToonOctahedralYShift   = 16U;
static const float k_ritchToonOctahedralMaxValue = 65535.0F;
static const float k_ritchToonOctahedralScale    = 2.0F;
static const float k_ritchToonOctahedralBias     = 1.0F;
static const float k_ritchToonOctahedralOne      = 1.0F;
static const float k_ritchToonOctahedralZero     = 0.0F;

// 接線の長さの2乗がこれ以下なら、接線を持たない頂点とみなす(C++側のModelOutlineNormalBuilderと同じ判定)
static const float k_ritchToonMinTangentLengthSquared = 1.0e-12F;

// 輪郭線の太さを、カメラからの距離に合わせて変える基準
// マテリアルの「輪郭線の太さ」は、カメラから1m離れて縦の画角60度で見たときの太さ(m)とする
// 画面の縦半分に映る範囲の高さ = 距離 × tan(縦の画角 / 2) なので、基準は 1m × tan(30度)
static const float k_ritchToonOutlineReferenceViewHalfHeight = 0.57735F;

// 距離に合わせた太さの倍率の範囲
// 近づくほど細く(画面上の太さを保つ)、離れるほど太くするが、
// ・近すぎるときに線が消えないよう、下限を持たせる
// ・遠くの小さなキャラクターが輪郭線で塗りつぶされないよう、上限を持たせる(遠くでは画面上で少しずつ細くなる)
static const float k_ritchToonOutlineMinWidthScale = 0.25F;
static const float k_ritchToonOutlineMaxWidthScale = 3.0F;

// 押し出した頂点を、カメラから見て奥へ下げる距離(押し出した距離に対する割合)
// 横から見ると輪郭線の形は変わらない(視線の上で動かすだけ)が、
// 腕と体の間、首の下などのくぼみで、輪郭線が手前の面の上に出てしまうのを防ぐ
static const float k_ritchToonOutlineDepthPushRatio = 1.0F;

// 他のMeshに覆われていない頂点(覆われている層の数が0)
static const uint k_ritchToonUncoveredLayerCount = 0U;

static const float k_ritchToonNoOutlineMask = 0.0F;

// 輪郭線の描画で、MeshShaderからPixelShaderへ渡す値
struct MSOutputRitchToonOutline
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;

    // カリングの可視化用の判定結果(k_modelCullingDebugState～。通常は0 = 見えている)
    nointerpolation uint cullingDebugState : CULLING_DEBUG_STATE;
};

// 面を照らすライトの色(ディレクショナルライト + 環境光)
// トゥーン調では「明るい所 / 影の所」の色をマテリアルの色で決めるため、
// ライトは全体の明るさと色味を決めるだけにする(表面と輪郭線で同じ明るさにするため、ここで共有する)
float3 CalculateRitchToonLightColor()
{
    const float3 l_directionalLightColor = g_directionalLightColor * g_directionalLightIntensity;
    const float3 l_ambientLightColor     = g_ambientLightColor     * g_ambientLightIntensity;

    return min(l_directionalLightColor + l_ambientLightColor, k_ritchToonMaxLightColor);
}

// Octahedral符号化でuint1つに詰めた向きを、長さ1のfloat3へ戻す
// (正八面体の上半分はxyをそのまま、下半分は四隅へ折り返したxyを元に戻す)
float3 DecodeRitchToonOctahedralNormal(const uint a_packedNormal)
{
    const float2 l_encoded = float2(a_packedNormal & k_ritchToonOctahedralLowMask, a_packedNormal >> k_ritchToonOctahedralYShift) / k_ritchToonOctahedralMaxValue * k_ritchToonOctahedralScale - k_ritchToonOctahedralBias;

    float3 l_normal = float3(l_encoded, k_ritchToonOctahedralOne - abs(l_encoded.x) - abs(l_encoded.y));

    // zが負(下半分)の場合だけ、折り返したxyを戻す
    const float l_fold = saturate(-l_normal.z);

    l_normal.x += l_normal.x >= k_ritchToonOctahedralZero ? -l_fold : l_fold;
    l_normal.y += l_normal.y >= k_ritchToonOctahedralZero ? -l_fold : l_fold;

    return normalize(l_normal);
}

// 法線だけから、法線に直交する接線と従法線を作る(Duffらの方法、2017年)
// 接線を持たない頂点で使う
// ※ C++側(ModelOutlineNormalBuilder::BuildOrthonormalBasis)と必ず同じ作り方にする
void BuildRitchToonOrthonormalBasis(const float3 a_normal, out float3 a_tangent, out float3 a_bitangent)
{
    const float l_sign = a_normal.z >= k_ritchToonOctahedralZero ? k_ritchToonOctahedralOne : -k_ritchToonOctahedralOne;
    const float l_a    = -k_ritchToonOctahedralOne / (l_sign + a_normal.z);
    const float l_b    = a_normal.x * a_normal.y * l_a;

    a_tangent   = float3(k_ritchToonOctahedralOne + l_sign * a_normal.x * a_normal.x * l_a, l_sign * l_b, -l_sign * a_normal.x);
    a_bitangent = float3(l_b, l_sign + a_normal.y * a_normal.y * l_a, -a_normal.y);
}

// 輪郭線を押し出す向き(スムーズ法線、World座標)
// 読み込み時に作った「頂点のTBNから見た向き」を、スキニング後のTBNで組み立て直す
// (FetchWorldNormalと同じTBNの作り方にして、読み込み時と同じ向きへ戻るようにしている)
// スムーズ法線のBufferが無い場合は、頂点の法線へ押し出す
float3 CalculateRitchToonOutlineDirection(const float3 a_worldNormal, const float4 a_worldTangent, const uint a_modelVertexIndex)
{
    const float3 l_worldNormal = normalize(a_worldNormal);

    if (g_outlineNormalBufferSRVDescriptorIndex == k_modelInvalidDescriptorIndex) { return l_worldNormal; }

    StructuredBuffer<uint> l_outlineNormalBuffer = ResourceDescriptorHeap[g_outlineNormalBufferSRVDescriptorIndex];

    const float3 l_tangentDirection = DecodeRitchToonOctahedralNormal(l_outlineNormalBuffer[a_modelVertexIndex]);

    // 接線を法線に対して直交化する(接線が無い頂点は、読み込み時と同じく法線だけから作る)
    const float3 l_rawTangent = a_worldTangent.xyz - l_worldNormal * dot(l_worldNormal, a_worldTangent.xyz);

    float3 l_worldTangent   = l_rawTangent;
    float3 l_worldBitangent = l_rawTangent;

    if (dot(l_rawTangent, l_rawTangent) <= k_ritchToonMinTangentLengthSquared)
    {
        BuildRitchToonOrthonormalBasis(l_worldNormal, l_worldTangent, l_worldBitangent);
    }
    else
    {
        l_worldTangent   = normalize(l_rawTangent);
        l_worldBitangent = cross(l_worldNormal, l_worldTangent) * a_worldTangent.w;
    }

    return normalize(l_worldTangent * l_tangentDirection.x + l_worldBitangent * l_tangentDirection.y + l_worldNormal * l_tangentDirection.z);
}

// 輪郭線マスク(R)を頂点のUVの位置から読む(黒い所は線を出さない)
float FetchRitchToonOutlineMask(const float2 a_uv, const RitchToonMaterial a_material)
{
    Texture2D<float4> l_outlineMaskTexture = ResourceDescriptorHeap[a_material.outlineMaskTextureSRVDescriptorIndex];

    // テクスチャの幅と高さ(ピクセル数)
    uint l_width     = k_ritchToonInitialTextureDimension;
    uint l_height    = k_ritchToonInitialTextureDimension;
    uint l_mipLevels = k_ritchToonInitialTextureDimension;

    l_outlineMaskTexture.GetDimensions(k_ritchToonOutlineMaskMipLevel, l_width, l_height, l_mipLevels);

    // UV(0.0F ~ 1.0F)を、何番目のピクセルかに変換する
    // UVが範囲外の場合も繰り返し(Wrap)として扱えるよう、frac()で小数部分だけにしてから掛ける
    const float2 l_textureSize = float2(l_width, l_height);
    const uint2  l_texelIndex  = min(uint2(frac(a_uv) * l_textureSize), uint2(l_width, l_height) - k_ritchToonLastTexelOffset);

    return l_outlineMaskTexture.Load(uint3(l_texelIndex, k_ritchToonOutlineMaskMipLevel)).r;
}

// 他のMeshに覆われている頂点か(服の下の肌など)
// 覆われている頂点を押し出すと、押し出した輪郭線が上に重なっている服を突き抜けて見えるため、押し出さない
// (覆われている層の数は、LODのためにモデルの読み込み時に求めてある物を使う)
bool IsRitchToonOutlineCoveredVertex(const uint a_modelVertexIndex)
{
    if (g_lodCoveredLayerCountBufferSRVDescriptorIndex == k_modelInvalidDescriptorIndex) { return false; }

    StructuredBuffer<uint> l_lodCoveredLayerCountBuffer = ResourceDescriptorHeap[g_lodCoveredLayerCountBufferSRVDescriptorIndex];

    return l_lodCoveredLayerCountBuffer[a_modelVertexIndex] != k_ritchToonUncoveredLayerCount;
}

// 輪郭線を描くために、頂点を外側へ押し出した位置(World座標)
// 押し出した形を「裏面だけ」描くと(背面法)、元の形の周りに太さの分だけはみ出した部分が残り、それが輪郭線に見える
//
// どんな場面でも不自然にならないための工夫
//   1. スムーズ法線へ押し出す         : 角ばった所でも押し出した面が裂けず、線が途切れない
//   2. 覆われている頂点は押し出さない   : 服の下の肌などの輪郭線が、服を突き抜けて見えない
//   3. カメラからの距離で太さを変える   : 近づいても線が太くなりすぎず、離れても消えない(上限、下限あり)
//   4. 押し出した頂点を奥へ下げる       : くぼみで輪郭線が手前の面の上に出ない
//   5. 輪郭線マスク                     : 顔の内側など、線を出したくない所に線を出さない
float3 ExtrudeRitchToonOutlineWorldPosition(const float3            a_worldPosition,
                                            const float3            a_worldNormal,
                                            const float4            a_worldTangent,
                                            const float2            a_uv,
                                            const uint              a_modelVertexIndex,
                                            const RitchToonMaterial a_material)
{
    const float l_outlineMask = IsRitchToonOutlineCoveredVertex(a_modelVertexIndex) ? k_ritchToonNoOutlineMask : FetchRitchToonOutlineMask(a_uv, a_material);

    // カメラから頂点までの奥行き(View空間のz)と、その奥行きで画面の縦半分に映る範囲の高さ
    const float l_viewDepth      = mul(float4(a_worldPosition, k_modelPositionElementW), g_viewMatrix).z;
    const float l_viewHalfHeight = max(l_viewDepth, g_nearClip) * g_tanHalfFOVY;

    // 映る範囲の高さに比例させると、画面上の太さがほぼ一定になる
    const float l_widthScale = clamp(l_viewHalfHeight / k_ritchToonOutlineReferenceViewHalfHeight, k_ritchToonOutlineMinWidthScale, k_ritchToonOutlineMaxWidthScale);

    const float l_extrudeDistance = a_material.outlineWidth * l_widthScale * l_outlineMask;

    const float3 l_outlineDirection = CalculateRitchToonOutlineDirection(a_worldNormal, a_worldTangent, a_modelVertexIndex);
    const float3 l_viewDirection    = normalize(a_worldPosition - g_cameraWorldPosition);

    return a_worldPosition + l_outlineDirection * l_extrudeDistance + l_viewDirection * (l_extrudeDistance * k_ritchToonOutlineDepthPushRatio);
}

#endif // MODEL_RITCH_TOON_HLSLI