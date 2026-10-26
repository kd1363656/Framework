#ifndef CAMERA_PASS_HLSLI
#define CAMERA_PASS_HLSLI

// ※ C++側(Definition/Struct/Graphics/Buffer/Constant/CBCameraPassStruct.h)と必ず同じ並びにする
cbuffer CBCameraPass : register(b0)
{
    row_major matrix g_viewMatrix;
    row_major matrix g_projectionMatrix;
    row_major matrix g_viewProjectionMatrix;
    
    float g_nearClip;
    float g_farClip;
    float g_tanHalfFOVX;
    float g_tanHalfFOVY;
    
    float3 g_cameraWorldPosition;
    float  g_cameraPassPadding;

    // カリング(Meshlet単位の視錐台、裏面の判定と、影のカスケードの選択)に使うカメラの値
    // 通常は上の描画に使うカメラと同じ値、エディターのデバッグカメラで描画している間だけメインカメラの値が入る
    // (デバッグカメラから見ても「メインカメラがどこまで描いているか」が分かるようにするため)
    row_major matrix g_cullingViewMatrix;

    float g_cullingNearClip;
    float g_cullingFarClip;
    float g_cullingTanHalfFOVX;
    float g_cullingTanHalfFOVY;

    // g_isCullingDebugVisualize : 1ならカリングで消された物も色を付けて描画する
    float3 g_cullingCameraWorldPosition;
    uint   g_isCullingDebugVisualize;
};

#endif // CAMERA_PASS_HLSLI
