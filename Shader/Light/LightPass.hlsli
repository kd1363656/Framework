#ifndef LIGHT_PASS_HLSLI
#define LIGHT_PASS_HLSLI

// ライトの定数(ディレクショナルライトと環境光)
// StandardLitとRitchToonなど、ライティングを行う全てのPixelShaderで共有する
// ※ C++側(Definition/Struct/Graphics/Buffer/Constant/CBLightStruct.h)と必ず同じ並びにする
cbuffer CBLightPass : register(b2)
{
    float3 g_directionalLightDirection;
    float  g_directionalLightIntensity;

    float3 g_directionalLightColor;
    float  g_lightPassPadding;

    float3 g_ambientLightColor;
    float  g_ambientLightIntensity;
};

#endif // LIGHT_PASS_HLSLI