#include "../../StaticModel.hlsli"

// o除算を避けるための小さい値。
static const float k_staticModelPBREpsilon = 0.00001F;

// PBR計算で使う基本定数。
static const float k_staticModelPBRZero  = 0.0F;
static const float k_staticModelPBROne   = 1.0F;
static const float k_staticModelPBRTwo   = 2.0F;
static const float k_staticModelPBRFour  = 4.0F;
static const float k_staticModelPBREight = 8.0F;

// 円周率
// Diffuseの積かやGGXの分母で使う。
static const float k_staticModelPBRPI = 3.14159265359F;

// Roughnessが完全に0になるとSpecularが極端に鋭くなり不安定になりやすい。
// そのため採点値を持たせる。
static const float k_staticModelMinimumroughness = 0.04F;

// 非金属の基本反射率。
// 多くの非金属はだいたい0.04付近。
static const float k_staticModelDefaultDielectricF0 = 0.04F;

// NormalMapは通常0.0F ~ 1.0Fで保存されている
// それを-1.0F ~ 1.0Fに戻すために使う
static const float k_staicModelNOrmalMapScale = k_staticModelPBRTwo;
static const float k_staicModelNOrmalMapBias  = k_staticModelPBROne;

// DisneyDiffuseで使う定数
// Rouhenessが高いほど、浅い角度でのDiffuseの変化が強くなる。
static const float k_staticModelDisneyDiffuseBaseFD90       = 0.5F;
static const float k_staticModelDisneyDiffuseRoughnessScale = k_staticModelPBRTwo;

// 同じ値を3成分のfloat3にする。
// float3(1, 1, 1)のようなマジックナンバー直書きを避けるために使う。
float3 CreateStaticModelFloat3(const float a_value)
{
    return float3(a_value, a_value, a_value);
}

// Xの5乗を求める。
// FresnelやDisneyDiffuseでよく使う。
float Pow5(const float a_value)
{
    const float l_value2 = a_value * a_value;
    
    return l_value2 * l_value2 * a_value;
}

// CookTorraceSpecular
// GGX / Trowbridge-Reitzの法線分布関数
// 表面の細かい面が、どれくらいHalfVector方向を向いているかを表す
float DistributionGGX(const float3 a_normal, const float3 a_halfVector, const float a_roughness)
{
    const float l_alpha       = a_roughness * a_roughness;
    const float l_alpha2      = l_alpha     * l_alpha;
    const float l_nDotH       = saturate(dot(a_normal, a_halfVector));
    const float l_nDotH2      = l_nDotH * l_nDotH;
    const float l_denominator = l_nDotH2 * (l_alpha2 - k_staticModelPBROne) + k_staticModelPBROne;

    return l_alpha2 / max(k_staticModelPBRPI * l_denominator * l_denominator, k_staticModelPBREpsilon);
}

// Schlick-GGXの幾何減衰
// Cook-TorranceのG項の一部
// 視線方向、またはライト方向から見たときに、細かい面がどれだけ隠れるかを表す
float GeometrySchlickGGX(const float a_nDotX, const float a_roughness)
{
    // 直接光用のSchlick-GGX近似
    const float l_roughness = a_roughness + k_staticModelPBROne;
    
    const float l_k = (l_roughness * l_roughness) / k_staticModelPBREight;
    
    return a_nDotX / max(a_nDotX * (k_staticModelPBROne - l_k) + l_k, k_staticModelPBREpsilon);
}

// Smith法の幾何減衰
// 視線方向とライト方向の両方を隠れ具合を掛け合わせる。
float GeometrySmith(const float3 a_normal, 
                    const float3 a_viewDirection,
                    const float3 a_lightDirection,
                    const float  a_roughness)
{
    const float l_nDotV         = saturate(dot(a_normal, a_viewDirection));
    const float l_nDotL         = saturate(dot(a_normal, a_lightDirection));
    const float l_geometryView  = GeometrySchlickGGX(l_nDotV, a_roughness);
    const float l_geometryLight = GeometrySchlickGGX(l_nDotL, a_roughness);

    return l_geometryView * l_geometryLight;
}

// SchlickFresnel
// Cook-TorranceのF項。
// 視線が浅い角度になるほど反射が強くなる。
float3 FresnelSchlick(const float a_cosTheta, const float3 a_f0)
{
    const float l_fresnel = Pow5(k_staticModelPBROne - saturate(a_cosTheta));

    return a_f0 + (CreateStaticModelFloat3(k_staticModelPBROne) - a_f0) * l_fresnel;
}

// DisneyDifffuse/BurleyDiffuse
// Lamberdiffuseを、視線角度・ライト角度・roughnessで少し自然に補正する
float DisneyDiffulseFactor(const float a_nDotV,
                           const float a_nDotL,
                           const float a_lDotH,
                           const float a_roughness)
{
    // FD90は浅い角度でのDifffuseの見え方を調整する値。
    const float l_fd90 = k_staticModelDisneyDiffuseBaseFD90       + 
                         k_staticModelDisneyDiffuseRoughnessScale * 
                         a_roughness                              * 
                         a_nDotL                                  *
                         a_lDotH;

    const float l_lightScatter = k_staticModelPBROne + (l_fd90 - k_staticModelPBROne) * Pow5(k_staticModelPBROne - a_nDotL);
    const float l_viewScatter  = k_staticModelPBROne + (l_fd90 - k_staticModelPBROne) * Pow5(k_staticModelPBROne - a_nDotV);
    
    return l_lightScatter * l_viewScatter;
}

// NormalMapを使ってWorld空間の法線を作る
// 頂点法線だけでは細かい凹凸が出ないため、NormalMapで法線方向を変える
float3 FetchStaticModelWorldNormal(const ModelMeshOutput a_input)
{
    Texture2D<float4> l_normalTexture = ResourceDescriptorHeap[g_normalTextureSRVDescriptorIndex];

    const float3 l_normal  = normalize(a_input.worldNormal);
    const float3 l_tangent = normalize(a_input.worldTangent.xyz);

    // TangentをNormalにして直行化する
    // 頂点データや補完の影響で、TangentとNormalが完全に直行していない場合がある
    const float3 l_orthogonalTangent = normalize(l_tangent - l_normal * dot(l_normal, l_tangent));
    
    // tangent.wはBitangentの向き補正
    const float3 l_bitangent = normalize(cross(l_normal, l_orthogonalTangent) * a_input.worldTangent.w);
    
    // Tangent空間からWorld空間へ変換するためのTBN行列
    const float3x3 l_tbn = float3x3(l_orthogonalTangent, l_bitangent, l_normal);

    // NormalMapは0.0F ~ 1.0Fなので、-1.0F ~ 1.0Fに戻す
    const float3 l_tangentNormal = l_normalTexture.Sample(g_textureSampler, a_input.uv).xyz * k_staicModelNOrmalMapScale - k_staicModelNOrmalMapBias;

    return normalize(mul(l_tangentNormal, l_tbn));
}

float4 main(const ModelMeshOutput a_input) : SV_Target0
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[g_baseColorTextureSRVDescriptorIndex];
    Texture2D<float4> l_metallicTexture  = ResourceDescriptorHeap[g_metallicTextureSRVDescriptorIndex];
    Texture2D<float4> l_roughnessTexture = ResourceDescriptorHeap[g_roughnessTextureSRVDescriptorIndex];
    
    const float4 l_baseColorSample = l_baseColorTexture.Sample(g_textureSampler, a_input.uv);
    
    // BaseColor
    const float3 l_baseColor = l_baseColorSample.rgb * g_baseColorFactor.rgb;
    
    // Metallic
    const float l_metallic = saturate(l_metallicTexture.Sample(g_textureSampler, a_input.uv).r * g_metallicFactor);

    // Roughness
    // 引くほど鏡面反射が鋭く、高いほどぼやける。
    const float l_roughness = max(saturate(l_roughnessTexture.Sample(g_textureSampler, a_input.uv).r * g_roughnessFactor), k_staticModelMinimumroughness);
    
    // NormalMap込みのWorld空間法線
    const float3 l_normal = FetchStaticModelWorldNormal(a_input);
    
    // Pixelからカメラへ向かう方向
    const float3 l_viewDirection = normalize(g_cameraWorldPosition - a_input.worldPosition);
    
    // g_directionalLightDirectionは光が進む方向として扱う
    // 面からライトへ向かう方向は逆向きなので-directionにする
    const float3 l_lightDirection = normalize(-g_directionalLightDirection);
    
    // HalfvectorはView方向とLight方向の中間方向
    const float3 l_halfVector = normalize(l_viewDirection + l_lightDirection);
    
    const float l_nDotL = saturate(dot(l_normal, l_lightDirection));    
    const float l_nDotV = saturate(dot(l_normal, l_viewDirection));
    const float l_lDotH = saturate(dot(l_lightDirection, l_halfVector));
    const float l_hDotV = saturate(dot(l_halfVector, l_viewDirection));

    // F0
    // 非金属は0.04F,金属はBaseColorが反対色になる
    const float3 l_f0 = lerp(CreateStaticModelFloat3(k_staticModelDefaultDielectricF0), l_baseColor, l_metallic);
    
    // Cook-TorranceSpecular
    const float l_distribution = DistributionGGX(l_normal, l_halfVector, l_roughness);
    
    const float l_geometry = GeometrySmith(l_normal, 
                                           l_viewDirection, 
                                           l_lightDirection,
                                           l_roughness);
    
    const float3 l_fresnel = FresnelSchlick(l_hDotV, l_f0);

    const float3 l_specularNumerator = l_distribution * l_geometry * l_fresnel;
    
    const float l_specularDenominator = max(k_staticModelPBRFour * l_nDotV * l_nDotL, k_staticModelPBREpsilon);
    
    const float3 l_cookTorranceSpecular = l_specularNumerator / l_specularDenominator;

    // DisneyDiffuse
    const float l_disneyDiffuseFactor = DisneyDiffulseFactor(l_nDotV, 
                                                             l_nDotL,
                                                             l_lDotH, 
                                                             l_roughness);
    
    const float3 l_disneyDiffuse = l_baseColor * l_disneyDiffuseFactor / k_staticModelPBRPI;

    // KSは鏡面反射成分
    const float3 l_ks = l_fresnel;
    
    // LDは拡散反射成分
    // 金属は拡散反射しないので、metallicが1に近いほどKDを減らす
    const float3 l_kd = (CreateStaticModelFloat3(k_staticModelPBROne) - l_ks) * (k_staticModelPBROne - l_metallic);
    
    const float3 l_radiance = g_directionalLightColor * g_directionalLightIntensity;
    
    // 直接光
    // DiffuseはDisney、SpecularはCook-Torrance
    const float3 l_directionLighting = (l_kd * l_disneyDiffuse + l_cookTorranceSpecular) * l_radiance * l_nDotL;

    // 簡易環境光
    // 本格的なIBLを入れるまでは、暗部を少し持ち上げる
    const float3 l_ambientLighting = g_ambientLightColor * g_ambientLightIntensity * l_baseColor;
    
    const float3 l_finalColor = l_directionLighting + l_ambientLighting;
    
    return float4(l_finalColor, l_baseColorSample.a * g_baseColorFactor.a);
}