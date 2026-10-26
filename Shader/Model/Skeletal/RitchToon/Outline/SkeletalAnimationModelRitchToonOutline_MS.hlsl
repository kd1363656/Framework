#include "../../../RitchToon/ModelRitchToon.hlsli"
#include "../../SkeletalAnimationModel.hlsli"

// SkeletalAnimationModelのRitchToonの輪郭線を描くMeshShader
// ComputeShaderがスキニングした頂点を読み、頂点を法線の方向へ輪郭線の太さだけ押し出して出力する
// (PipelineStateで表面側をカリングするため、押し出した形の裏面だけが描かれ、それが輪郭線に見える)
[outputtopology("triangle")]
[numthreads(k_modelMeshShaderThreadCountX, k_modelMeshShaderThreadCountY, k_modelMeshShaderThreadCountZ)]
void main(in  payload  ModelAmplificationPayload a_payload,
          out vertices MSOutputRitchToonOutline  a_vertexList[k_modelMAXMeshletVertexCount],
          out indices  uint3                     a_primitiveList[k_modelMAXMeshletPrimitiveCount],
              const    uint3                     a_groupID : SV_GroupID,
              const    uint                      a_groupThreadIndex : SV_GroupIndex)
{
    StructuredBuffer<SkeletalAnimationSkinnedVertex> l_skinnedVertexBuffer     = ResourceDescriptorHeap[g_vertexBufferSRVDescriptorIndex];
    StructuredBuffer<ModelMeshlet>                   l_modelMeshletBuffer      = ResourceDescriptorHeap[g_meshletBufferSRVDescriptorIndex];
    StructuredBuffer<uint>                           l_uniqueVertexIndexBuffer = ResourceDescriptorHeap[g_uniqueVertexIndexBufferSRVDescriptorIndex];

    // 輪郭線の太さとマスクはMaterialが持っている
    const RitchToonMaterial l_material = FetchRitchToonMaterial(g_materialBufferSRVDescriptorIndex);

    // ASからPayload経由で渡された値には、Meshletの番号と、カリングの可視化用の判定結果が入っている
    const uint         l_packedMeshletIndex = a_payload.meshletIndexList          [a_groupID.x];
    const uint         l_meshletIndex       = DecodeModelCullingDebugMeshletIndex(l_packedMeshletIndex);
    const uint         l_cullingDebugState  = SelectModelCullingDebugState        (g_cullingDebugState, DecodeModelCullingDebugState(l_packedMeshletIndex));
    const ModelMeshlet l_modelMeshlet       = l_modelMeshletBuffer                [l_meshletIndex];

    // 出力頂点数、三角形数を設定
    SetMeshOutputCounts(l_modelMeshlet.vertexCount, l_modelMeshlet.triangleCount);

    for (uint l_vertexIndex = a_groupThreadIndex; l_vertexIndex < l_modelMeshlet.vertexCount; l_vertexIndex += k_modelMeshShaderThreadCountX)
    {
        // Meshlet内の頂点番号から、元の頂点Bufferの頂点番号を取得する
        const uint l_modelVertexIndex = l_uniqueVertexIndexBuffer[l_modelMeshlet.vertexOffset + l_vertexIndex];

        const SkeletalAnimationSkinnedVertex l_skinnedVertex = l_skinnedVertexBuffer[l_modelVertexIndex];

        // ワールド座標、法線、接線を計算し、輪郭線を押し出す向き(スムーズ法線)へ押し出す
        const float3 l_worldPosition = ApplyModelLODCoveredLayerDepthOffset(TransformModelLocalPositionToWorld(l_skinnedVertex.position), l_modelVertexIndex);
        const float3 l_worldNormal   = TransformModelLocalNormalToWorld    (l_skinnedVertex.normal);
        const float4 l_worldTangent  = TransformModelLocalTangentToWorld   (l_skinnedVertex.tangent);

        const float3 l_outlineWorldPosition = ExtrudeRitchToonOutlineWorldPosition(l_worldPosition,
                                                                                   l_worldNormal,
                                                                                   l_worldTangent,
                                                                                   l_skinnedVertex.uv,
                                                                                   l_modelVertexIndex,
                                                                                   l_material);

        a_vertexList[l_vertexIndex].position          = mul(float4(l_outlineWorldPosition, k_modelPositionElementW), g_viewProjectionMatrix);
        a_vertexList[l_vertexIndex].uv                = l_skinnedVertex.uv;
        a_vertexList[l_vertexIndex].cullingDebugState = l_cullingDebugState;
    }

    for (uint l_triangleIndex = a_groupThreadIndex; l_triangleIndex < l_modelMeshlet.triangleCount; l_triangleIndex += k_modelMeshShaderThreadCountX)
    {
        // 3個Pack方式のPrimitiveIndexから、三角形1個分の頂点番号を取り出す
        a_primitiveList[l_triangleIndex] = FetchModelPackedPrimitiveIndex(l_modelMeshlet.triangleOffset + l_triangleIndex);
    }
}