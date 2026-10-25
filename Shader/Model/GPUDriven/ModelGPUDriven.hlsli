#ifndef MODEL_GPU_DRIVEN_HLSLI
#define MODEL_GPU_DRIVEN_HLSLI

// GPUドリブン描画で使用する構造体
// ※ C++側(Definition/Struct/Graphics/Buffer/Structured/ModelGPUDrivenStruct.h)と必ず同じ並びにする

// CPUから送られる、モデル1体分(インスタンス)の情報
struct ModelGPUDrivenInstance
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;

    // animationFrameIndex : スキニングするモデルの場合に使う、事前計算したアニメーションの何コマ目か
    // morphWeightOffset   : このインスタンスだけのモーフのウェイトが、上書き用ウェイトBufferの何番目から始まるか
    //                       無効値なら、ベイクしたウェイト(アニメーションのコマ番号の位置)を使う
    float worldMaxScale;
    float worldOrientationSign;
    uint  animationFrameIndex;
    uint  morphWeightOffset;
};

// CPUから送られる、Mesh1つ分の情報(モデル内の全インスタンスで共通)
struct ModelGPUDrivenMeshInfo
{
    float3 boundsCenter;
    float  boundsRadius;

    // materialBufferSRVDescriptorIndex : MaterialのGPUDataBuffer
    // Materialの値(色やテクスチャ)はMaterialの種類ごとに違うため、ここには持たせずMaterial側のBufferを参照する
    uint materialBufferSRVDescriptorIndex;
    uint vertexBufferSRVDescriptorIndex;
    uint meshletBufferSRVDescriptorIndex;
    uint uniqueVertexIndexBufferSRVDescriptorIndex;

    uint primitiveIndexBufferSRVDescriptorIndex;
    uint meshletBoundsBufferSRVDescriptorIndex;
    uint lodOffset;
    uint lodCount;

    uint allMeshletCount;
    uint allTriangleCount;
    uint skinningMatrixBufferSRVDescriptorIndex;
    uint skinningPaletteBase;

    // スキニングするモデルの場合に使う値
    // スキニング行列の番号 = アニメーションのコマ番号 × skinningPaletteStride + skinningPaletteBase + ボーンパレット番号
    uint skinningPaletteStride;
    uint skinningFrameCount;
    uint morphVertexRangeBufferSRVDescriptorIndex;
    uint morphDeltaBufferSRVDescriptorIndex;

    // モーフを持つMeshの場合に使う値(持たないMeshではmorphDeltaBufferSRVDescriptorIndexが無効)
    // morphChannelListBufferSRVDescriptorIndex : このMeshが使うチャンネル番号の一覧(morphChannelListCount個)
    // morphWeightBufferSRVDescriptorIndex      : ベイクしたウェイト(ウェイトの番号 = コマ番号 × morphWeightStride + チャンネル番号)
    uint morphChannelListBufferSRVDescriptorIndex;
    uint morphChannelListCount;
    uint morphWeightBufferSRVDescriptorIndex;
    uint morphWeightStride;

    // morphLODTargetErrorBufferSRVDescriptorIndex : LODごと・ターゲットごとのモーフの崩れ
    //                                               (番号 = LOD番号 × morphChannelListCount + ターゲット番号)
    // lodSurfaceDeviationScale                    : LODで表面が実際にずれた量 ÷ LODの誤差 の最大値(覆われている頂点を奥へ押し出す距離に掛ける)
    // lodCoveredLayerCountBufferSRVDescriptorIndex : 頂点ごとの覆われている層の数のBuffer
    // padding2 : C++側と大きさを16byte単位で揃えるための詰め物
    uint  morphLODTargetErrorBufferSRVDescriptorIndex;
    float lodSurfaceDeviationScale;
    uint  lodCoveredLayerCountBufferSRVDescriptorIndex;
    uint  padding2;
};

// LOD1段階分の情報(C++のStruct::ModelMeshLODと同じ)
struct ModelGPUDrivenMeshLOD
{
    uint  meshletOffset;
    uint  meshletCount;
    uint  triangleCount;
    float simplifyError;

    // 全ターゲットの中で、ウェイト1で効かせたときに一番大きい崩れ(モーフを持たないMeshは0)
    // LODを選ぶときは、ターゲットごとの崩れ(morphLODTargetErrorBuffer)を使う
    float morphError;
};

// 描画1回分の定数バッファの中身
// AS / MS / PSが読むcbuffer CBModelPerObject(Model.hlsli)と全く同じ並びで書き込み、
// そのままConstantBufferViewとして使う
// 定数バッファの開始位置は256byte単位にする決まりがあるため、256byteになるようにPaddingを入れている
struct ModelGPUDrivenDrawRecord
{
    row_major float4x4 worldMatrix;
    row_major float4x4 worldInverseTransposeMatrix;

    uint materialBufferSRVDescriptorIndex;
    uint vertexBufferSRVDescriptorIndex;
    uint meshletBufferSRVDescriptorIndex;
    uint uniqueVertexIndexBufferSRVDescriptorIndex;

    uint  primitiveIndexBufferSRVDescriptorIndex;
    uint  meshletBoundsBufferSRVDescriptorIndex;
    float worldMaxScale;
    float worldOrientationSign;

    uint  meshletCount;
    uint  meshletOffset;
    float lodCoveredLayerDepthOffset;
    uint  lodCoveredLayerCountBufferSRVDescriptorIndex;

    uint skinningMatrixBufferSRVDescriptorIndex;
    uint skinningMatrixOffset;
    uint isGPUSkinning;
    uint skinningPadding;

    uint morphVertexRangeBufferSRVDescriptorIndex;
    uint morphDeltaBufferSRVDescriptorIndex;
    uint morphWeightBufferSRVDescriptorIndex;
    uint morphWeightOffset;

    uint4 recordPadding[3];
};

// ExecuteIndirectが1回分の描画として読む引数
// C++側のCommandSignatureで「定数バッファのアドレス → DispatchMeshの引数」の順と決めている
struct ModelGPUDrivenIndirectCommand
{
    uint2 constantBufferAddress;
    uint3 dispatchMeshArguments;
    uint  padding;
};

#endif // MODEL_GPU_DRIVEN_HLSLI