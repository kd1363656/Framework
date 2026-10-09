#ifndef STATIC_MODEL_HLSLI
#define STATIC_MODEL_HLSLI

struct StaticModelVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 uv;

    // アウトライン用の平滑化法線(同じ位置の頂点が同じ向きを持つ法線)
    // 陰影の計算には使わず、アウトラインで頂点を押し出す方向にだけ使う
    float3 smoothedNormal;
};

#endif // STATIC_MODEL_HLSLI