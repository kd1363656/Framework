#include "../../StaticModel.hlsli"

// 0除算を避けるための小さな値
// NdotVやNdotLが0に近いと、Cook-Torranceの分母が0に近くなるため、
// max()でこの値を下限として扱う
static const float k_minCookTorranceDenominator = 0.00001F;

static const float k_pi = 3.14159265359F;

// Roughnessが完全に0になると、Specularが極端に鋭くなり不安定になりやすい
// そのため最低値を持たせる
static const float k_minRoughnessForSpecularStability = 0.04F;

// 非金属の基本反射率
// 多くの非金属は真正面から見た反射率が大体0.04付近
static const float k_defaultDielectricF0 = 0.04F;

// NormalMapは通常0.0F ~ 1.0Fで保存されている
// PBR計算では-1.0F ~ 1.0Fの方向ベクトルとして使いたいので、
// normal * 2.0F - 1.0Fで-1.0F ~ 1.0Fの値を取得する
static const float k_normalMapStoredRangeScale = 2.0F;
static const float k_normalMapStoredRangeBias  = 1.0F;

// Cook-TorranceSpecularの分母で使う係数。
// MicrofacetBRDFの式では、Specular = D * G * F / (4 * NdotV * NdotL)になる
static const float k_cookTorranceDenominatorScale = 4.0F;

// 直接光用Schlick-GGX Geometry項のK計算で使う分母
// k = ((roughness + 1) ^ 2) / 8
static const float k_schlickGGXDirectLightKDenominator = 8.0F;

// DisneyDiffuse / BurleyDiffuseで使うFD90の基本値
// FD90は、浅い角度でのDiffuse補正に使う値
static const float k_disneyDiffuseBaseFD90 = 0.5F;

// DisneyDiffuse / BurleyDiffuseで、roughnessが浅い角度のDiffuseに与える影響の強さ
static const float k_disneyDiffuseRoughnessScale = 2.0F;

static const float k_ggxNormalDistributionDenominatorBase = 1.0F;

// 直接光用Schlick-GGXGeometry項では、
// roughnessに1.0Fを足してからK値を計算する
// k = ((roughness + 1) ^ 2) / 8の1に相当する
static const float k_schlickGGXDirectLightRoughnessBias = 1.0F;

// Schlik-GGXGeometry項では、
// roughnessに1.0Fを足してからK値を計算する。
// k = ((roughness + 1) ^ 2) / 8の1に相当する
static const float k_schlickGGXGeometryMaxVisibility = 1.0F;

// 引数の5乗の値を返す
float CalculatePow5(const float a_value)
{
    const float l_value2 = a_value * a_value;
    
    return l_value2 * l_value2 * a_value;
}

// Cook-TorranceSpecularのD項
// GGX / Trowbridge-Reitzの法線分布関数
// D項は、表面上の細かい面、つまりMicrofacetが、
// HalfVector方向をどれくらい向いているか表す
// Roughnessが低い場合 : Microfacetの向きがそろいやすい
//                       HalfVector方向を向く面が狭い範囲に集中する
//                       そのため、ハイライトが小さく鋭くなる
// Roughnessが高い場合 : Microfacetの向きがばらつく
//                       反射が広い範囲に散る
//                       そのため、ハイライトが大きくぼやける
float CalculateGGXNormalDistribution(const float3 a_normal, const float3 a_halfVector, const float a_roughness)
{
    // alphaはGGX計算用に変換したroughness
    // roughnessはそのまま使うより、2乗した値を使うことで、
    // roughnessの変化が見た目に自然に反映されやすくなる。
    const float l_alpha = a_roughness * a_roughness;
    
    // alpha ^ 2
    // GGXの式ではalphaの2乗を使う
    const float l_alpha2 = l_alpha * l_alpha;
    
    // NormalとHalfVectorがどれくらい同じ方向を向いているか。
    // 1.0に近い : 法線がHalfVector方向を向いている。
    //             反射に都合が良い
    // 0.0に近い : 法線がHalfVector方向を向いていない。
    //             反射に都合が悪い
    const float l_normalDotHalf  = saturate(dot(a_normal, a_halfVector));
    const float l_normalDotHalf2 = l_normalDotHalf * l_normalDotHalf;
    
    // GGX法線分布関数の分母
    // denominator = NdotH ^ 2 * (alpha ^ 2 - 1) + 1
    // alphaが小さい、つまりroughnessが低いと
    // NdotHが1に近い場所だけ値が大きくなり、鋭いハイライトになる
    // alphaが大きい、つまりroughnessが高いと、
    // 値が広がり、ぼやけたハイライトになる
    const float l_denominator = l_normalDotHalf2 * (l_alpha2 - k_ggxNormalDistributionDenominatorBase) + k_ggxNormalDistributionDenominatorBase;

    // GGXのD項。
    // PI * denominator ^ 2が0に近くなると不安定なので、
    // max()で下限値を持たせる
    return l_alpha2 / max(k_pi * l_denominator * l_denominator, k_minCookTorranceDenominator);
}

// Cook-TorranceSpecularのG項で使うSchlick-GGX近似
// G項は、表面の細かい凹凸によって、光や視線がどれくらい隠れるかを表す。
// View方向またはLight方向片方だけに対して、
// どれくらい隠れていないかを計算する
// a_normalDotDirection;
// NdotVまたはNdotLを渡す。
// NdotV = NormalとViewDirectionの内積
// NdotL = NormalとLightDirectionの内積
// 戻り値 : 1.0に近いほど、Microfacetが隠れていない。
//          0.0に近いほど、Microfacetが凹凸によって隠れている
float CalculateSchlickGGXGeometry(const float a_normalDotDirection, const float a_roughness)
{
    // 直接光用のSclick-GGX近似では、
    // roughnessに1を足してから2乗し、8で割ってK値を作る。
    // k = ((roughness + 1) ^ 2) / 8
    // このK値は、roughnessに応じてGeometryの減衰具合を確認するために使う。
    const float l_roughnessWithBias = a_roughness + k_schlickGGXDirectLightRoughnessBias;
    const float l_k                 = (l_roughnessWithBias * l_roughnessWithBias) / k_schlickGGXDirectLightKDenominator;
    
    // Schilick-GGXGeometry項。
    // normalDotDirectionが大きい : 面がView方向またはLight方向を向いている,
    //                              隠れにくい
    // normalDotDirectionが小さい : 浅い角度から見ている、または浅い角度から光が当たっている。 
    //                              凹凸によって隠れやすい
    return a_normalDotDirection / max(a_normalDotDirection * (k_schlickGGXGeometryMaxVisibility - l_k) + l_k, k_minCookTorranceDenominator);
}

// Cook-TorranceSpecularのG項
// Smith法のGeometry項。
// Schlick-GGXGeometryを,View方向とLight方向の両方に対して計算し、
// それらを掛け合わせる。
// なぜ2つ必要か : ライトから光が届いてもカメラから見えなければ反射は見えない
//                 カメラから見えていても、ライトが届かなければ反射は出ない
// そのため、View側の見えやすさ * Light側の届きやすさで最終的なGeometry項を作る
float CalculateSmithGeometry(const float3 a_normal, 
                             const float3 a_viewDirection,
                             const float3 a_lightDirection,
                             const float a_roughness)
{
    // NormalとViewDirectionがどれくらい同じ方向を向いているか。
    // 1.0に近いほど正面から見ている。
    // 0.0に近いほど浅い角度から見ている
    const float l_normalDotView = saturate(dot(a_normal, a_viewDirection));

    // NormalとLightDirectionがどれくらい同じ方向を向いているか
    // 1.0に近いほどライトが正面から当たっている
    // 0.0に近いほどライトが浅い角度から当たっている
    const float l_normalDotLight = saturate(dot(a_normal, a_lightDirection));
    
    // カメラから見たときのMicrofacetの隠れ具合
    const float l_viewGeometry = CalculateSchlickGGXGeometry(l_normalDotView, a_roughness);

    // ライトから見たときのMicrofacetの隠れ具合
    const float l_lightGeometry = CalculateSchlickGGXGeometry(l_normalDotLight, a_roughness);

    // View側とLight側の両方が成立して初めて反射が見える
    return l_viewGeometry * l_lightGeometry;
}

float4 main(const ModelMeshOutput a_input) : SV_Target0
{
 
}