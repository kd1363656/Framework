#ifndef MODEL_STANDARD_LIT_HLSLI
#define MODEL_STANDARD_LIT_HLSLI
#include "../ModelStandard.hlsli"

struct MSOutputLit
{
    float4 position      : SV_Position;
    float3 worldPosition : POSITION0;
    float3 worldNormal   : NORMAL0;
    float4 worldTangent  : TANGENT0;
    float2 uv            : TEXCOORD0;

    // カリングの可視化用の判定結果(k_modelCullingDebugState～。通常は0 = 見えている)
    // 三角形の中で補間すると意味が無くなるため、nointerpolationで代表頂点の値をそのまま使う
    nointerpolation uint cullingDebugState : CULLING_DEBUG_STATE;
};

// NormalMapは通常0.0F ~ 1.0Fで保存されている
// 法線の計算では-1.0F ~ 1.0Fの方向ベクトルとして使いたいので、
// normal * 2.0F - 1.0Fで-1.0F ~ 1.0Fの値を取得する
static const float k_normalMapStoredRangeScale = 2.0F;
static const float k_normalMapStoredRangeBias  = 1.0F;

// NormalMapの凹凸をそのまま使う強さ(1.0Fで100%、0.0Fで頂点法線のまま)
static const float k_modelFullNormalMapStrength = 1.0F;

// NormalMapを使ってWorld空間の法線を作る。
// 頂点法線だけでは、面の細かい凹凸を表現できない。
// NormalMapからTangent空間の法線を読み取り、
// TBN行列でWorld空間の法線へ変換する
// a_normalStrength : 凹凸の強さ。0.0Fに近いほど凹凸が弱くなり、頂点法線に近づく
//                    (トゥーン調では凹凸が強いと影の境界がザラつくため、弱めて使う)
float3 FetchWorldNormal(const MSOutputLit a_input, const uint a_normalTextureSRVDescriptorIndex, const float a_normalStrength)
{
    Texture2D<float4> l_normalTexture = ResourceDescriptorHeap[a_normalTextureSRVDescriptorIndex];

    // MeshShaderから渡されたWorld空間の法線
    // 補間後の値なので、念のためnormalizeする
    const float3 l_worldNormal = normalize(a_input.worldNormal);

    // MeshShaderから渡されたWorld空間のTangent
    // こちらも補間後の値なのでnormalizeする
    const float3 l_worldTangent = normalize(a_input.worldTangent.xyz);

    // TangentをNormalに対して直交化する
    // 頂点データやラスタライズ時の補間の影響で
    // TangentとNormalが完全に直交していない場合がある。
    // そのままTBN行列を作るとNormalMapの向きが歪むため、
    // TangentからNormal方向の成分を取り除いて、直交したTangentにする
    const float3 l_orthogonalWorldTangent = normalize(l_worldTangent - l_worldNormal * dot(l_worldNormal, l_worldTangent));

    // Bitangentを作る
    // a_input.worldTangent.wはBitangentの向きを補正
    // ミラーUVなどでBitangentの向きが反転する場合があるため、
    // tangent.wをかけて向きを補正する
    const float3 l_worldBitangent = normalize(cross(l_worldNormal, l_orthogonalWorldTangent) * a_input.worldTangent.w);

    // Tangent空間からWorld空間へ変換するためのTBN行列
    // T = Tangent
    // B = Bitangent
    // N = Normal
    const float3x3 l_tangentToWorldMatrix = float3x3(l_orthogonalWorldTangent, l_worldBitangent, l_worldNormal);

    // NormalMapは通常0.0F ~ 1.0Fで保存されている
    // しかし、法線方向として使うには-1.0F ~ 1.0Fに戻す必要がある。
    // 0.0F -> -1.0F
    // 0.5F ->  0.0F
    // 1.0F ->  1.0F
    float3 l_tangentNormal = l_normalTexture.Sample(g_textureSampler, a_input.uv).xyz * k_normalMapStoredRangeScale - k_normalMapStoredRangeBias;

    // Tangent空間のxyは「面がどれだけ傾いているか」を表す
    // ここを小さくすると傾きが弱まり、凹凸が控えめになる(zはそのままなので、normalizeで長さを1に戻す)
    l_tangentNormal.xy *= a_normalStrength;

    // Tangent空間のNormalMap法線をWorld空間へ変換する
    return normalize(mul(normalize(l_tangentNormal), l_tangentToWorldMatrix));
}

#endif // MODEL_STANDARD_LIT_HLSLI