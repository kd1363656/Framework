#include "../../../Model.hlsli"
#include "../../SkeletalAnimationModel.hlsli"
#include "../../../Shadow/Cascade/ModelCascadeShadow.hlsli"

[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices MSOutputCascadeShadow     a_vertexList[k_modelMAXMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
              const    uint3                     a_groupID : SV_GroupID,
              const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    // この描画のオブジェクト(行列)とメッシュ(バッファの番号)を、テーブルから読む
    const ModelObjectData l_object = FetchModelObjectData();
    const ModelMeshData   l_mesh   = FetchModelMeshData  ();

    StructuredBuffer<SkeletalAnimationSkinnedVertex> l_skinnedVertexBuffer      = ResourceDescriptorHeap[l_mesh.vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>                   l_modelMeshletBuffer       = ResourceDescriptorHeap[l_mesh.meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>                           l_uniqueVertexIndexBuffer  = ResourceDescriptorHeap[l_mesh.uniqueVertexIndexBufferSRVDescriptorIndex];

    // 現在の子MeshShaderGroup番号を使って、
    // AmplificationShaderがPayload配列へ格納した可視MeshletIndexを取得する
    const uint l_meshletIndex = a_payload.meshletIndexList[a_groupID.x];

    const ModelMeshlet l_modelMeshlet = l_modelMeshletBuffer[l_meshletIndex];

    // 現在のMeshletが出力する頂点数と三角形数を設定する
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);

    // MeshShaderGroup内のThreadで、
    // Skinning済みの頂点の変換を分担する
    for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        const uint l_uniqueVertexIndex = l_modelMeshlet.vertexOffset + l_vertexIndex;
        const uint l_modelVertexIndex  = l_uniqueVertexIndexBuffer[l_uniqueVertexIndex];

        // ComputeShaderで現在Poseへ変形済みの頂点を取得する
        const SkeletalAnimationSkinnedVertex l_skinnedVertex = l_skinnedVertexBuffer[l_modelVertexIndex];

        // SkinnedVertexのPositionはModelLocal空間なので、
        // ModelのWorldMatrixでWorld空間へ変換する
        const float3 l_worldPosition = TransformModelLocalPositionToWorld(l_object, l_skinnedVertex.position);

        // World座標を現在のCascadeに対応する
        // LightClip空間へ変換する
        a_vertexList[l_vertexIndex].position = mul(float4(l_worldPosition, k_modelPositionElementW), g_cascadeViewProjectionMatrix);
    }

    // MeshShaderGroup内のThreadで、
    // Meshletが持つ三角形Indexの出力を分担する
    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        const uint l_packedPrimitiveIndex = l_modelMeshlet.triangleOffset + l_triangleIndex;

        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_object, l_mesh, l_packedPrimitiveIndex);
    }
}