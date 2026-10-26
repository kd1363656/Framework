#include "ModelRitchToon.hlsli"
#include "../Standard/Lit/ModelStandardLit.hlsli"
#include "../Shadow/Cascade/CascadeShadowMapPass.hlsli"

// RitchToon(トゥーン調)の表面を描くPixelShader
// MeshShaderはStandardLitと同じ物を使う(ワールド座標、法線、接線、UVがあれば足りるため)
//
// 色の決め方
//   1. 影         : 光の当たり具合をくっきりした段差(明るい / 影)にし、影の所を影色にする
//   2. 光沢       : メタリックマスクの所だけ、ライトのハイライトをくっきりした形で足す
//   3. マットキャップ : 法線の向きで「ツヤの絵(球の絵)」から色を読み、マスクの所だけ足す
//   4. 発光       : 発光マスクの所だけ、ライトに関係なく色を足す

// Half-Lambert(NdotL * 0.5 + 0.5)の係数
// NdotLは-1.0F ~ 1.0Fなので、0.0F ~ 1.0Fへ並べ直してから影の境界と比べる
// (真横から光が当たる所が0.5になり、影の境界を0.0F ~ 1.0Fの値で指定しやすくなる)
static const float k_ritchToonHalfLambertScale = 0.5F;
static const float k_ritchToonHalfLambertBias  = 0.5F;

// 影が全く無い(光が完全に当たっている)状態
static const float k_ritchToonFullLitFactor = 1.0F;

static const float3 k_ritchToonWhiteColor = float3(1.0F, 1.0F, 1.0F);

// smoothstepは「下限と上限が同じ」だと0除算になるため、ぼかしの幅はこの値より小さくしない
static const float k_ritchToonMinBlur = 0.0001F;

// 光沢の鋭さ(ハイライトの小ささ)
// smoothness(0.0F ~ 1.0F)を、pow()に渡す指数1 ~ 2048(2の0乗 ~ 2の11乗)へ変換する
static const float k_ritchToonMaxSpecularPowerExponent = 11.0F;

// 光沢のくっきりした形を作るための境界とぼかし(ハイライトの形をトゥーン調の段差にする)
static const float k_ritchToonSpecularBorder = 0.5F;
static const float k_ritchToonSpecularBlur   = 0.05F;

// マットキャップのUVは-1.0F ~ 1.0Fの方向から作るため、0.0F ~ 1.0Fへ並べ直す
static const float k_ritchToonMatcapUVScale = 0.5F;
static const float k_ritchToonMatcapUVBias  = 0.5F;

// 0.0F ~ 1.0Fの値を、くっきりした段差(0.0F / 1.0F)にする
// a_border : 段差の位置
// a_blur   : 段差の境界をぼかす幅(0に近いほどくっきり)
float CalculateRitchToonStep(const float a_value, const float a_border, const float a_blur)
{
    const float l_blur = max(a_blur, k_ritchToonMinBlur);

    return smoothstep(a_border - l_blur, a_border + l_blur, a_value);
}

// マットキャップのテクスチャを読むUVを作る
// マットキャップは「カメラから見た法線の向き」で、球の絵のどこを読むかを決める
// 法線をView空間へ変換したxyをそのまま使うと、画面の端ほど(斜めから見るほど)絵が歪む
// そこで「カメラから頂点への向き」と法線の外積を使い、見る向きの分を打ち消してからUVにする
float2 CalculateRitchToonMatcapUV(const float3 a_worldPosition, const float3 a_worldNormal)
{
    // 法線は方向なのでw = 0、位置はw = 1でView空間へ変換する
    const float3 l_viewNormal    = normalize(mul(float4(a_worldNormal,   k_modelDirectionElementW), g_viewMatrix).xyz);
    const float3 l_viewDirection = normalize(mul(float4(a_worldPosition, k_modelPositionElementW),  g_viewMatrix).xyz);

    // カメラの正面(viewDirection = (0, 0, 1))で見たとき、外積は(-normal.y, normal.x, 0)になる
    // そのためyをU、xをVとして使うと、法線の右向きがUの右、上向きがVの上(テクスチャの上)になる
    const float3 l_crossed = cross(l_viewDirection, l_viewNormal);

    return float2(l_crossed.y, l_crossed.x) * k_ritchToonMatcapUVScale + k_ritchToonMatcapUVBias;
}

float4 main(const MSOutputLit a_input) : SV_Target0
{
    // このMeshのMaterial(トゥーン描画に必要な値とテクスチャ)
    const RitchToonMaterial l_material = FetchRitchToonMaterial(g_materialBufferSRVDescriptorIndex);

    Texture2D<float4> l_shadowMaskTexture   = ResourceDescriptorHeap[l_material.shadowMaskTextureSRVDescriptorIndex];
    Texture2D<float4> l_matcapTexture       = ResourceDescriptorHeap[l_material.matcapTextureSRVDescriptorIndex];
    Texture2D<float4> l_matcapMaskTexture   = ResourceDescriptorHeap[l_material.matcapMaskTextureSRVDescriptorIndex];
    Texture2D<float4> l_metallicMaskTexture = ResourceDescriptorHeap[l_material.metallicMaskTextureSRVDescriptorIndex];
    Texture2D<float4> l_emissionMaskTexture = ResourceDescriptorHeap[l_material.emissionMaskTextureSRVDescriptorIndex];

    // メインテクスチャの色
    const float4 l_baseColor = FetchModelBaseColor(l_material.baseColorTextureSRVDescriptorIndex, l_material.baseColorFactor, a_input.uv);

    // NormalMap込みのWorld空間法線(強さはマテリアルで弱められる)
    const float3 l_normal = FetchWorldNormal(a_input, l_material.normalTextureSRVDescriptorIndex, l_material.normalStrength);

    // g_directionalLightDirectionは光が進む方向として扱う
    // 面からライトへ向かう方向は逆向きなので、-directionにする
    const float3 l_lightDirection = normalize(-g_directionalLightDirection);
    const float3 l_viewDirection  = normalize(g_cameraWorldPosition - a_input.worldPosition);

    // ---------------------------------------------------------------
    // 1. 影
    // ---------------------------------------------------------------
    // 光の当たり具合(0.0F = 真裏、0.5F = 真横、1.0F = 正面)
    const float l_halfLambert = dot(l_normal, l_lightDirection) * k_ritchToonHalfLambertScale + k_ritchToonHalfLambertBias;

    // 他の物の影(CascadeShadowMap)の中なら暗くする(1.0Fで光が届いている、0.0Fで影の中)
    const float l_cascadeShadowVisibility = CalculateCascadeShadowMapVisibility(a_input.worldPosition);

    // 光の当たり具合を、影の境界でくっきり分ける(1.0Fで明るい所、0.0Fで影の所)
    const float l_toonLitFactor = CalculateRitchToonStep(l_halfLambert * l_cascadeShadowVisibility, l_material.shadowBorder, l_material.shadowBlur);

    // 影マスク(R)が黒い所は影を出さない(鼻や口の周りなど、影が出ると汚く見える所に使う)
    const float l_shadowMask = l_shadowMaskTexture.Sample(g_textureSampler, a_input.uv).r;
    const float l_litFactor  = lerp(k_ritchToonFullLitFactor, l_toonLitFactor, l_shadowMask);

    // 明るい所はメインテクスチャの色のまま、影の所はメインテクスチャの色 × 影色にする
    const float3 l_albedo = l_baseColor.rgb * lerp(l_material.shadowColor, k_ritchToonWhiteColor, l_litFactor);

    const float3 l_lightColor = CalculateRitchToonLightColor();

    float3 l_finalColor = l_albedo * l_lightColor;

    // ---------------------------------------------------------------
    // 2. 光沢
    // ---------------------------------------------------------------
    // HalfVector(視線とライトの中間の向き)と法線が近いほど、ライトが目に向かって反射する
    const float3 l_halfVector       = normalize(l_viewDirection + l_lightDirection);
    const float  l_normalDotHalf    = saturate(dot(l_normal, l_halfVector));
    const float  l_specularPower    = exp2(l_material.smoothness * k_ritchToonMaxSpecularPowerExponent);
    const float  l_specularShape    = pow(l_normalDotHalf, l_specularPower);
    const float  l_toonSpecular     = CalculateRitchToonStep(l_specularShape, k_ritchToonSpecularBorder, k_ritchToonSpecularBlur) * l_litFactor;

    // メタリックマスク(R) : 光沢を出す所と強さ
    // 金属らしい所ほど、ハイライトの色をメインテクスチャの色に寄せる(金属は自身の色で光を反射するため)
    const float  l_metallicMask = l_metallicMaskTexture.Sample(g_textureSampler, a_input.uv).r;
    const float3 l_specularTint = lerp(l_material.specularColor, l_material.specularColor * l_baseColor.rgb, l_metallicMask);

    l_finalColor += l_specularTint * l_toonSpecular * l_metallicMask * l_lightColor;

    // ---------------------------------------------------------------
    // 3. マットキャップ
    // ---------------------------------------------------------------
    // マットキャップの絵(黒地にツヤの絵)を、マスク(R)の所だけ足す(黒い所は何も足されない)
    const float2 l_matcapUV    = CalculateRitchToonMatcapUV(a_input.worldPosition, l_normal);
    const float3 l_matcapColor = l_matcapTexture.Sample(g_textureSampler, l_matcapUV).rgb;
    const float  l_matcapMask  = l_matcapMaskTexture.Sample(g_textureSampler, a_input.uv).r;

    l_finalColor += l_matcapColor * l_material.matcapColor * l_matcapMask * l_lightColor;

    // ---------------------------------------------------------------
    // 4. 発光
    // ---------------------------------------------------------------
    // 発光マスク(RGB)の所だけ、ライトに関係なく色を出す(暗い所でも光って見える)
    // メインテクスチャの色を混ぜると、髪の毛先が髪の色のまま光るように、場所ごとに違う色で光らせられる
    const float3 l_emissionMask  = l_emissionMaskTexture.Sample(g_textureSampler, a_input.uv).rgb;
    const float3 l_emissionColor = l_material.emissionColor * lerp(k_ritchToonWhiteColor, l_baseColor.rgb, l_material.emissionBaseColorBlend);

    // 色の出し方は2通りを割合で混ぜる
    // ・足す       : 元の色の上に発光色を足す(元の色が残り、明るく光る)
    // ・置き換える : 発光マスクの濃さの分だけ、元の色を発光色へ置き換える
    //               マスクが根元から毛先へ黒 → 白のグラデーションなら、髪の色から発光色へのグラデーションになる
    const float3 l_addedColor    = l_finalColor + l_emissionColor * l_emissionMask;
    const float3 l_replacedColor = lerp(l_finalColor, l_emissionColor, l_emissionMask);

    l_finalColor = lerp(l_addedColor, l_replacedColor, l_material.emissionReplaceRate);

    // カリングの可視化中は、消されたはずのMesh / Meshletに判定結果ごとの色を混ぜる(通常はそのままの色)
    return ApplyModelCullingDebugTint(float4(l_finalColor, l_baseColor.a), a_input.cullingDebugState);
}