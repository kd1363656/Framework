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

// Schlick-GGXGeometry項で使う最大可視率
// Geometry項は、Microfacetがどれくらい隠れていないかを0.0F ~ 1.0Fで表す
// 1.0Fは「完全に隠れていない」状態。
static const float k_schlickGGXGeometryMaxVisibility = 1.0F;

// SchlickFresnelで使う最大反射率
// F = F0 + (1 - F0) * pow(1 - cosTheta, 5)の1に相当する
// 1.0Fは「完全反射」に近い最大値として使う。
static const float k_fresnelMaxReflectance = 1.0F;

// DisneyDiffuseの基準散乱値
// lightScatter / viewScatterの基本値として使う。
// 1.0Fは「補正なしのDiffuse倍率」を表す。
static const float k_disneyDiffuseBaseScatter = 1.0F;

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
    // 直接光用のSchlick-GGX近似では、
    // roughnessに1を足してから2乗し、8で割ってK値を作る。
    // k = ((roughness + 1) ^ 2) / 8
    // このK値は、roughnessに応じてGeometryの減衰具合を確認するために使う。
    const float l_roughnessWithBias = a_roughness + k_schlickGGXDirectLightRoughnessBias;
    const float l_k                 = (l_roughnessWithBias * l_roughnessWithBias) / k_schlickGGXDirectLightKDenominator;
    
    // Schlick-GGXGeometry項。
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

// Cook-TorranceSpecularのF項
// SchlickFresnel近似
// Fresnelは、視線が浅い角度になるほど反射が強くなる現象
// たとえば、水面や床は、正面から見るより斜めから見える方が反射が強く見える
// a_cosTheta : HalfVectorとViewDirectionの内積を渡す。
//              値が1.0に近いほど正面寄り
//              値が0.0に近いほど浅い角度
// a_f0 : 正面から見たときの基本反射率
//        非金属なら大体0.04。
//        金属ならBaseColorを使う。
float3 CalculateSchlickFresnel(const float a_cosTheta, const float3 a_f0)
{
    // 1 - cosTheta
    // cosThetaが1.0に近い、つまり正面から見えているときは0に近くなる。
    // cosThetaが0.0に近い、つまり浅い角度から見ているときは1に近くなる
    const float l_inverseCosTheta = k_fresnelMaxReflectance - saturate(a_cosTheta);
    
    // SchlickFresnelでは(1 - cosTheta) ^ 5を使う
    // 5乗にすることで、正面では変化が小さく
    // 浅い角度で急に反射が強くなる
    const float l_fresnelFactor = CalculatePow5(l_inverseCosTheta);

    // 最大反射率をfloat3にする
    const float3 l_maxReflectance = float3(k_fresnelMaxReflectance, k_fresnelMaxReflectance, k_fresnelMaxReflectance);

    // SchlickFresnelの式
    // F = F0 + (1 - F0) * (1 - cosTheta) ^ 5
    // 正面から見る     : FresnelFactorが0に近いので、FはF0に近い
    // 浅い角度から見る : FresnelFactorが1に近づくので、Fは1.0に近づく
    return a_f0 + (l_maxReflectance - a_f0) * l_fresnelFactor;
}

// DisneyDiffuse / BurleyDiffuse
// LambertDiffuseを、視線角度・ライト角度・roughnessで自然に補正する
// 戻り値は、baseColor / PI に掛けるDiffuse補正係数。
// つまり、この関数自体は色ではなく「Diffuseの倍率」を返す。
// a_normalDotView : NdotV。
//                   NormalとViewDirectionの内積。
//                   1.0に近いほど正面から見ている。
//                   0.0に近いほど浅い角度から見ている。
//
// a_normalDotLight : NdotL。
//                    NormalとLightDirectionの内積。
//                    1.0に近いほどライトが正面から当たっている。
//                    0.0に近いほど浅い角度からライトが当たっている。
//
// a_lightDotHalf : LdotH。
//                  LightDirectionとHalfVectorの内積。
//                  DisneyDiffuseでは、浅い角度でのDiffuse補正に使う。
//
// a_roughness : 表面の粗さ。
//               roughnessが高いほど、浅い角度でのDiffuse変化が強くなる
float CalculateDisneyDiffuseFactor(const float a_normalDotView,
                                   const float a_normalDotLight,
                                   const float a_lightDotHalf,
                                   const float a_roughness)
{
    // LdotHの2乗
    // BurleyDiffuseでは、FD90の計算にLdotH ^ 2を使う。
    const float l_lightDotHalf2 = a_lightDotHalf * a_lightDotHalf;
    
    // FD90はライトや視線が浅い角度になった時のDiffuse補正に使う
    // roughnessが高い : 浅い角度でのDiffuse変化が強くなる。
    // LdotHが大きい   : ライト方向とHalfVectorが近く、補正が強くなる。 
    const float l_fd90 = k_disneyDiffuseBaseFD90       + 
                         k_disneyDiffuseRoughnessScale * 
                         a_roughness                   *
                         l_lightDotHalf2;
    
    // ライト方向に対する散乱補正。
    // NdotLが1.0に近い、つまりライトが正面から当たるときは補正が弱い
    // NdotLが0.0に近い、つまり浅い角度からあたるときは補正が強くなる。
    const float l_lightScatter = k_disneyDiffuseBaseScatter + (l_fd90 - k_disneyDiffuseBaseScatter) * CalculatePow5(k_disneyDiffuseBaseScatter - a_normalDotLight);

    // 視線方向に対する散乱補正
    // NdotVが1.0に近い、つまり正面から見るときは補正が弱い
    // NdotVが0.0に近い、つまり浅い角度から見るときは補正が強くなる
    const float l_viewScatter = k_disneyDiffuseBaseScatter + (l_fd90 - k_disneyDiffuseBaseScatter) * CalculatePow5(k_disneyDiffuseBaseScatter - a_normalDotView);

    // ライト側と視線側の補正を掛け合わせて、最終的なDiffuse補正係数にする。
    return l_lightScatter * l_viewScatter;
}

// NormalMapを使ってWorld空間の法線を作る。
// 頂点法線だけでは、面の細かい凹凸を表現できない。
// NormalMapからTangent空間の法線を読み取り、
// TBN行列でWorld空間の法線へ変換する
float3 FetchWorldNormal(const ModelMeshOutput a_input)
{
    Texture2D<float4> l_normalTexture = ResourceDescriptorHeap[g_normalTextureSRVDescriptorIndex];
    
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
    const float3 l_tangentNormal = l_normalTexture.Sample(g_textureSampler, a_input.uv).xyz * k_normalMapStoredRangeScale - k_normalMapStoredRangeBias;
    
    // Tangent空間のNormalMap法線をWorld空間へ変換する
    return normalize(mul(l_tangentNormal, l_tangentToWorldMatrix));
}

float4 main(const ModelMeshOutput a_input) : SV_Target0
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVDescriptorIndex];
    Texture2D<float4> l_metallicTexture  = ResourceDescriptorHeap[g_metallicTextureSRVDescriptorIndex];
    Texture2D<float4> l_roughnessTexture = ResourceDescriptorHeap[g_roughnessTextureSRVDescriptorIndex];
    
    // BaseColorTextureを読む
    // rgbは色、aは透明度として使う
    const float4 l_baseColorSample = l_baseColorTexture.Sample(g_textureSampler, a_input.uv);
    
    // BaseColor、非金属ではDiffuse色として使う
    // 金属ではSpecular反射色として使う
    const float3 l_baseColor = l_baseColorSample.rgb * g_baseColorFactor.rgb;
    
    // Metallic
    // 0.0Fに近いほど非金属、1.0Fに近いほど金属。
    const float l_metallic = saturate(l_metallicTexture.Sample(g_textureSampler, a_input.uv).r * g_metallicFactor);
    
    // Roughness
    // 低いほど鏡面反射が鋭く、高いほどぼやける
    // 0に近すぎるとSpecularが不安定になりやすいので最低値を持たせる
    const float l_roughness = max(saturate(l_roughnessTexture.Sample(g_textureSampler, a_input.uv).r * g_roughnessFactor), k_minRoughnessForSpecularStability);

    // NormalMap込みのWorld空間法線
    const float3 l_normal = FetchWorldNormal(a_input);

    // Pixelからカメラへ向かう方向
    const float3 l_viewDirection = normalize(g_cameraWorldPosition - a_input.worldPosition);

    // g_directionalLightDirectionは光が進む方向として扱う
    // 面からライトへ向かう方向は逆向きなので、-directionにする
    const float3 l_lightDirection = normalize(-g_directionalLightDirection);
    
    // HalfVectorはView方向とLight方向の中間方向
    // Microfacetがこの方向を向いていると、ライトから来た光がカメラへ反射しやすい
    const float3 l_halfVector = normalize(l_viewDirection + l_lightDirection);
    
    // PBRで使う角度情報
    const float l_normalDotLight = saturate(dot(l_normal,         l_lightDirection));
    const float l_normalDotView  = saturate(dot(l_normal,         l_viewDirection));
    const float l_lightDotHalf   = saturate(dot(l_lightDirection, l_halfVector));
    const float l_halfDotView    = saturate(dot(l_halfVector,     l_viewDirection));
    
    // F0。
    // 非金属では0.04付近。
    // 金属ではBaseColorが反射色になる
    const float3 l_f0 = lerp(float3(k_defaultDielectricF0, k_defaultDielectricF0, k_defaultDielectricF0), l_baseColor, l_metallic);
    
    // Cook-TorranceSpecularのD項
    // HalfVector方向を向いたMicrofacetがどれくらいあるかを表す。
    const float l_normalDistribution = CalculateGGXNormalDistribution(l_normal, l_halfVector, l_roughness);
    
    // Cook-TorranceSpecularのG項
    // MicrofacetがView方向とLight方向からどれくらい隠れていないかを表す
    const float l_geometry = CalculateSmithGeometry(l_normal, 
                                                    l_viewDirection, 
                                                    l_lightDirection,
                                                    l_roughness);
    
    // Cook-TorranceSpecularのF項
    // 視線が浅い角度になるほど反射が強くなるFresnelを表す
    const float3 l_fresnel = CalculateSchlickFresnel(l_halfDotView, l_f0);
    
    // Cook-Torranceの分子
    // Specular = D * G * F / (4 * NdotV * NdotL)
    const float3 l_specularNumerator = l_normalDistribution * l_geometry * l_fresnel;
    
    // Cook-Torranceの分母。
    // NdotVやNdotLが0に近いと分母が0に近づくため、下限値を持たせる
    const float l_specularDenominator = max(k_cookTorranceDenominatorScale * l_normalDotView * l_normalDotLight, k_minCookTorranceDenominator);
    
    // Cook-TorranceSpecular
    // 表面のMicrofacetによる鏡面反射
    const float3 l_cookTorranceSpecular = l_specularNumerator / l_specularDenominator;
    
    // DisneyDiffuseの補正係数
    const float l_disneyDiffuseFactor = CalculateDisneyDiffuseFactor(l_normalDotView,
                                                                     l_normalDotLight,
                                                                     l_lightDotHalf,
                                                                     l_roughness);
    
    // DisneyDiffuse
    // baseColor / PIが基本のLambertDiffuse
    // そこにDisneyDiffuseの角度補正を掛ける
    const float3 l_disneyDiffuse = l_baseColor * l_disneyDiffuseFactor / k_pi;

    // Specularに使う割合
    // Fresnelは、浅い角度ほどSpecularを強くする
    const float3 l_specularRatio = l_fresnel;
    
    // Diffuseに使う割合
    // エネルギー保存のため、Specularに使った分はDiffuseから減らす
    // さらに、金属は拡散反射しないので、metallicが高いほどDiffuseを減らす
    const float3 l_diffuseRatio = (float3(k_fresnelMaxReflectance, k_fresnelMaxReflectance, k_fresnelMaxReflectance) - l_specularRatio) * (k_fresnelMaxReflectance - l_metallic);
    
   // DirectionalLightの放射輝度の簡易表現
    // 現段階では、ライト色 * ライト強度として扱う
    const float3 l_radiance = g_directionalLightColor * g_directionalLightIntensity;
    
    // 直接光
    // DiffuseはDisneyDiffuse、SpecularはCook-TorranceSpecular
    // 最後にNdotLを掛けることで、ライトが正面から当たるほど明るくなる
    const float3 l_directionLighting = (l_diffuseRatio          * 
                                        l_disneyDiffuse         + 
                                        l_cookTorranceSpecular) * 
                                        l_radiance              * 
                                        l_normalDotLight;
    
    // 簡易環境光
    const float3 l_ambientLighting = g_ambientLightColor * g_ambientLightIntensity * l_baseColor;
    
    // 最終色
    const float3 l_finalColor = l_directionLighting + l_ambientLighting;
    
    return float4(l_finalColor, l_baseColorSample.a * g_baseColorFactor.a);
}