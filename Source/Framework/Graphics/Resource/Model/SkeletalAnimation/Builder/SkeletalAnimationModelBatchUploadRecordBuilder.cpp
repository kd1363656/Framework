#include "SkeletalAnimationModelBatchUploadRecordBuilder.h"

bool FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::CreateSkeletalAnimationModelBatchUploadRecord(const Device&                                                   a_device,
	                                                                                                              const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
	                                                                                                                    std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
	                                                                                                                    TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool,
	                                                                                                                    SkeletalAnimationModelRecord&                             a_skeletalAnimationModelRecord) const
{
	auto& l_modelMeshList = a_skeletalAnimationModelRecord.GetMutableREFModelData().m_modelMeshList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、SkeletalAnimationModelBatchUploadの作成に失敗しました。", false);

	for (auto& l_modelMesh : l_modelMeshList)
	{
		if (l_modelMesh.m_bonePaletteList.empty()) 
		{
			// 前のModelMeshまでに作成済みのBufferを解放する
			ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

			FWK_ASSERT_RETURN_VALUE("BonePaletteListが空のため、SkeletalAnimationModelBatchUploadの作成に失敗しました。", false);
		}

		// 共通5BufferとBonePaletteBufferのすべてが完成するまで、
		// ModelMesh本体のRuntimeDataへは反映しない
		SkeletalAnimationModelRecord::ModelMeshRuntimeData l_modelMeshRuntimeData = {};

		// Vertex、Meshlet、MeshletBoundsの共通Bufferを作成する
		if (!m_batchUploadRecordBuilder.CreateModelMeshBatchUploadRecord(a_device,
																		 a_gpuMemoryAllocator,
																		 l_modelMesh,
																		 a_bufferUploadCommandList,
																		 a_cbvSRVUAVDescriptorPool,
																		 l_modelMeshRuntimeData))
		{
			// 現在作成中のl_modelMeshRuntimeDataは、
			// この関数を抜ける際に自動的にReleaseされる。
			// ここでは前のModelMeshまでに完成しているBufferを解放する
			ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

			FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelの共通MeshBuffer作成に失敗しました。", false);
		}

		// SkeletalAnimationModel二のみ存在するBonePaletteBufferを作成する
		if (!l_modelMeshRuntimeData.m_bonePaletteBuffer.Create(l_modelMesh.m_bonePaletteList,
			                                                   a_device,
															   a_gpuMemoryAllocator,
															   a_bufferUploadCommandList,
															   a_cbvSRVUAVDescriptorPool))
		{
			// l_modelMeshRuntimeData内の共通Bufferは、
			// 関数を抜ける際に自動的にReleaseされる
			ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

			FWK_ASSERT_RETURN_VALUE("BonePaletteBuffer用StructuredBufferの作成に失敗しました。", false);
		}

		// 共通5BufferとBonePaletteBufferがすべて完成したら、
		// 派生RuntimeData全体をModelMeshへ移動する
		l_modelMesh.m_modelMeshRuntimeData = std::move(l_modelMeshRuntimeData);
	}

	return true;
}

void FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::ReleaseCreatedSkeletalAnimationModelStructuredBuffer(std::vector<SkeletalAnimationModelRecord::ModelMesh>& a_modelMeshList) const
{
	for (auto& l_modelMesh : a_modelMeshList)
	{
		auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;

		// StaticとSkeletalで共通する5種類のBufferを解放する
		m_batchUploadRecordBuilder.ReleaseModelMeshRuntimeData(l_modelMeshRuntimeData);

		// SkeletalAnimationModel固有のBonePaletteBufferを解放する
		l_modelMeshRuntimeData.m_bonePaletteBuffer.Release();
	}
}