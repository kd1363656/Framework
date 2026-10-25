#ifndef MODEL_STANDARD_HLSLI
#define MODEL_STANDARD_HLSLI
#include "../Model.hlsli"
#include "../../Camera/CameraPass.hlsli"

SamplerState g_textureSampler : register(s0);

// 他のMeshに覆われている頂点を、覆われている層の数だけカメラから見て奥へ押し出す(World座標)
//
// LODは三角形を減らすときに曲面を弦のように横切るため、表面が元の位置から少しずれる。
// 服が肌に密着していると、服が肌より奥へ沈んで、深度の比べ合いに負けて肌が透けて見えてしまう。
// そこで覆われている頂点を「LODの許容誤差 × 覆われている層の数」だけ奥へ押し出す(肌はInnerの奥、Innerはスカートの奥…)
// 覆う側が許容誤差の分だけ沈んでも、覆われる側はさらにその奥になるので、深度の比べ合いの結果(前後関係)が崩れない。
//
// カメラからの視線の上で前後に動かしても、画面に映る位置は1ピクセルも変わらない(変わるのは深度だけ)ため、
// 形や輪郭は元のまま。外側に何も無い頂点は動かさない
float3 ApplyModelLODCoveredLayerDepthOffset(const float3 a_worldPosition, const uint a_modelVertexIndex)
{
    // 押し出す距離が0(Bufferが無い場合など)なら、頂点の情報を読まずに済ませる
    if (g_lodCoveredLayerDepthOffset == k_modelEmptyLODCoveredLayerDepthOffset ||
        g_lodCoveredLayerCountBufferSRVDescriptorIndex == k_modelInvalidDescriptorIndex)
    {
        return a_worldPosition;
    }

    StructuredBuffer<uint> l_lodCoveredLayerCountBuffer = ResourceDescriptorHeap[g_lodCoveredLayerCountBufferSRVDescriptorIndex];

    const uint l_coveredLayerCount = min(l_lodCoveredLayerCountBuffer[a_modelVertexIndex], k_modelMaxLODCoveredLayerCount);

    // カメラから頂点へ向かう向き(視線の向き)へ押し出す
    const float3 l_viewDirection = normalize(a_worldPosition - g_cameraWorldPosition);

    return a_worldPosition + l_viewDirection * (g_lodCoveredLayerDepthOffset * (float)l_coveredLayerCount);
}

// StaticModelのLocal法線をWorld空間へ変換する
// 法線は位置ではなく方向なのでw = 0
// 非均一スケールでも法線方向が壊れにくいようにWorldInverseTransposeMatrixを使う。
float3 TransformModelLocalNormalToWorld(const float3 a_localNormal)
{
    const float4 l_localNormal = float4(a_localNormal, k_modelDirectionElementW);
    const float4 l_worldNormal = mul   (l_localNormal, g_worldInverseTransposeMatrix);
    
    return normalize(l_worldNormal.xyz);
}

// StaticModelのLocalTangentをWorld空間へ変換する
// Tangentは方向なのでw = 0。
// tangent.wはNormalMap用の向き補正なので維持する
float4 TransformModelLocalTangentToWorld(const float4 a_localTangent)
{
    const float4 l_localTangent = float4(a_localTangent.xyz, k_modelDirectionElementW);
    const float4 l_worldTangent = mul   (l_localTangent,     g_worldMatrix);

    return float4(normalize(l_worldTangent.xyz), a_localTangent.w * g_worldOrientationSign);
}

// ベースカラーテクスチャの色に係数を掛けた色を取得する
// テクスチャと係数はMaterialの種類ごとのデータから渡す
float4 FetchModelBaseColor(const uint a_baseColorTextureSRVDescriptorIndex, const float4 a_baseColorFactor, const float2 a_uv)
{
    Texture2D<float4> l_baseColorTexture = ResourceDescriptorHeap[a_baseColorTextureSRVDescriptorIndex];

    const float4 l_baseColorSample = l_baseColorTexture.Sample(g_textureSampler, a_uv);

    return l_baseColorSample * a_baseColorFactor;
}

#endif // MODEL_STANDARD_HLSLI