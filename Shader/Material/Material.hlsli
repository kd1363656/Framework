#ifndef MATERIAL_HLSLI
#define MATERIAL_HLSLI

// MaterialのGPUDataBufferは、Material1つ分のデータだけを持つ1要素のStructuredBuffer
// C++側では描画するMaterialの型を知らなくても、このBufferの番号だけをシェーダーへ渡す
// シェーダー側は自身が扱うMaterialの型として読む
static const uint k_materialBufferElementIndex = 0U;

#endif // MATERIAL_HLSLI