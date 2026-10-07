#ifndef CULLING_CAMERA_PASS_HLSLI
#define CULLING_CAMERA_PASS_HLSLI

cbuffer CBCullingCameraPass : register(b4)
{
    row_major matrix g_cullingViewMatrix;

    float g_cullingNearClip;
    float g_cullingFarClip;
    float g_cullingTanHalfFOVX;
    float g_cullingTanHalfFOVY;

    float3 g_cullingCameraWorldPosition;
    float  g_cullingCameraPassPadding;
};

#endif // CULLING_CAMERA_PASS_HLSLI