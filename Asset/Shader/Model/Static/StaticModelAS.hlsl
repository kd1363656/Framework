#include "StaticModel.hlsli"

// 指定したMeshletがFrustum内にあるか判定する
bool IsVisibleStaticModelMeshletByFrustum(const uint a_meshletIndex)
{
    // MeshletごとのBoundingSphere情報を読む。
    StructuredBuffer<ModelMeshletBounds> l_meshletBoundsBuffer = ResourceDescriptorHeap[g_meshletBoundsBufferSRVDescriptorIndex];

}

// Model共通のAmplificationShader
[numthreads(k_modelAmplificationShaderThreadCountX, k_modelAmplificationShaderThreadCountY, k_modelAmplificationShaderThreadCountZ)]
void main(uint3 a_groupID : SV_GroupID)
{
    ModelAmplificationPayload l_payload;
    
    // CPU側が(meshletCount, 1, 1)を呼びため、
    // ASのa_groupID.xはそのままMeshletIndexとして使える。
    l_payload.meshletIndex = a_groupID.x;
    
    DispatchMesh(k_modelAmplificationDispatchMeshGroupCountX, k_modelAmplificationDispatchMeshGroupCountY, k_modelAmplificationDispatchMeshGroupCountZ, l_payload);
}