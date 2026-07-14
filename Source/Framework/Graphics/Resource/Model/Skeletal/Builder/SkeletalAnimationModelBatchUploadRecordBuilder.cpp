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

		// SkeletalAnimationModelにのみ存在するBonePaletteBufferを作成する
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

	// Skeleton、BindPose、Motion、Track、KeyFrameは、
	// Mesh単位ではなくSkeletalAnimationModel Asset全体で共有する。
	// 全MeshのBuffer作成が成功した後に1組だけ作成する
	if (!CreateSkeletalAnimationModelSharedStructuredBuffer(a_device,
															a_gpuMemoryAllocator,
															a_bufferUploadCommandList,
															a_cbvSRVUAVDescriptorPool,
															a_skeletalAnimationModelRecord))
	{
		// 共有Bufferはすべてローカルで作成してからRecordへ反映するため、
		// 失敗した時点ではRecordに不完全な共有Bufferは残っていない。
		// ここでは既に作成済みのMesh単位Bufferを解放する
		ReleaseCreatedSkeletalAnimationModelStructuredBuffer(l_modelMeshList);

		FWK_ASSERT_RETURN_VALUE("SkeletalAnimationModelの共有StructuredBuffer作成に失敗しました。", false);
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::CreateSkeletalAnimationModelSharedStructuredBuffer(const Device&                                                   a_device, 
	                                                                                                                   const GPUMemoryAllocator&                                       a_gpuMemoryAllocator, 
	                                                                                                                         std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList, 
	                                                                                                                         TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool, 
	                                                                                                                         SkeletalAnimationModelRecord&                             a_skeletalAnimationModelRecord) const
{
	auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

	StructuredBufferSourceData l_structuredBufferSourceData = {};

	// CPU側ではBone、Motion、Track、KeyFrameが複数の可変長vectorに分かれている。
	// Compute ShaderからIndexだけで直接参照できるように、
	// GPUへ送る前に4本の一次元配列へ変換する
	FWK_ASSERT_RETURN_VALUE_IF(!BuildSkeletalAnimationModelStructuredBufferSourceData(l_modelData,
		                                                                              l_structuredBufferSourceData),
		                                                                              "SkeletalAnimationModel用StructuredBufferSourceDataの作成に失敗しました。",
		                                                                              false);

	// 共有Buffer作成の途中で失敗した場合に、
	// 呼び出し元のUploadCommandListへ不完全なCommandを残さないように、
	// 一度ローカルのCommandListへ作成する
	std::vector<StaticStructuredBuffer::BufferUploadCommand> l_sharedBufferUploadCommandList = {};

	// Recordへ途中まで作成したBufferを反映しないように、
	// 4個のBufferをまずローカル変数として作成する。
	// 途中で失敗した場合は、作成済みのローカルBufferが自動的に解放される
	StaticStructuredBuffer l_boneBuffer            = {};
	StaticStructuredBuffer l_motionSequenceBuffer  = {};
	StaticStructuredBuffer l_boneMotionTrackBuffer = {};
	StaticStructuredBuffer l_keyFrameBuffer        = {};

	// Bone BufferにはBindPoseも格納されている。
	// Motionを持たないモデルをBindPoseで扱う場合にも必要なため、
	// Bone Bufferだけは必ず作成する
	FWK_ASSERT_RETURN_VALUE_IF(!l_boneBuffer.Create(l_structuredBufferSourceData.m_boneBufferElementList,
		                                            a_device,
		                                            a_gpuMemoryAllocator,
		                                            l_sharedBufferUploadCommandList,
		                                            a_cbvSRVUAVDescriptorPool),
		                                            "SkeletalAnimationModel用BoneBufferの作成に失敗しました。",
		                                            false);

	// Motionが存在しない場合、Motion、Track、KeyFrameの配列は空になる。
	// StaticStructuredBufferは空配列から作成できないため、
	// Motionが存在する場合だけ残りの3個を作成する
	if (!l_structuredBufferSourceData.m_motionSequenceBufferElementList.empty())
	{
				// Motionごとの再生時間、FrameRate、
		// BoneMotionTrack配列の先頭IndexをGPUへ送る
		FWK_ASSERT_RETURN_VALUE_IF(!l_motionSequenceBuffer.Create(l_structuredBufferSourceData.m_motionSequenceBufferElementList,
			                                                      a_device,
			                                                      a_gpuMemoryAllocator,
			                                                      l_sharedBufferUploadCommandList,
			                                                      a_cbvSRVUAVDescriptorPool),
			                                                      "SkeletalAnimationModel用MotionSequenceBufferの作成に失敗しました。",
			                                                      false);

		// MotionごとにBone数と同じ数のTrack要素をGPUへ送る。
		// これによりCompute ShaderではTrackの検索処理が不要になる
		FWK_ASSERT_RETURN_VALUE_IF(!l_boneMotionTrackBuffer.Create(l_structuredBufferSourceData.m_boneMotionTrackBufferElementList,
			                                                       a_device,
			                                                       a_gpuMemoryAllocator,
			                                                       l_sharedBufferUploadCommandList,
			                                                       a_cbvSRVUAVDescriptorPool),
			                                                       "SkeletalAnimationModel用BoneMotionTrackBufferの作成に失敗しました。",
			                                                       false);

		// 全Motionの全Trackが参照するKeyFrameを、
		// 1本の連続したStructuredBufferとしてGPUへ送る
		FWK_ASSERT_RETURN_VALUE_IF(!l_keyFrameBuffer.Create(l_structuredBufferSourceData.m_keyFrameBufferElementList,
			                                                a_device,
			                                                a_gpuMemoryAllocator,
			                                                l_sharedBufferUploadCommandList,
			                                                a_cbvSRVUAVDescriptorPool),
			                                                "SkeletalAnimationModel用KeyFrameBufferの作成に失敗しました。",
			                                                false);
	}

	// 必要なBufferがすべて作成できたため、
	// この時点で初めてRecordへ共有GPU Resourceを移動する
	a_skeletalAnimationModelRecord.ApplySharedStructuredBuffers(l_structuredBufferSourceData.m_maxBoneHierarchyDepth,
																std::move(l_boneBuffer),
																std::move(l_motionSequenceBuffer),
																std::move(l_boneMotionTrackBuffer),
															    std::move(l_keyFrameBuffer));

	// 共有Bufferの作成がすべて成功した場合だけ、
	//　呼びだし元のBatchUploadCommandへ追加する
	for (auto& l_sharedBufferUploadCommand : l_sharedBufferUploadCommandList)
	{
		a_bufferUploadCommandList.emplace_back(std::move(l_sharedBufferUploadCommand));
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelBatchUploadRecordBuilder::BuildSkeletalAnimationModelStructuredBufferSourceData(SkeletalAnimationModelRecord::ModelData& a_modelData, StructuredBufferSourceData& a_structuredBufferSourceData) const
{
	      auto& l_modelBoneList           = a_modelData.m_boneList;
	const auto& l_modelMotionSequenceList = a_modelData.m_motionSequenceList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneList.empty(), "ModelBoneListが空のため、SkeletalAnimationModel用StructuredBufferSourceDataの作成に失敗しました。", false);

	const auto& l_maxStructuredBufferElementCount = static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max());
	const auto& l_boneCount                       = l_modelBoneList.size                                            ();
	const auto& l_motionSequenceCount             = l_modelMotionSequenceList.size                                  ();

	FWK_ASSERT_RETURN_VALUE_IF(l_boneCount > l_maxStructuredBufferElementCount,           "Bone数がuint32_tで表現できる範囲を超えており、SkeletalAnimationModel用StructuredBufferSourceDataの作成に失敗しました。",           false);
	FWK_ASSERT_RETURN_VALUE_IF(l_motionSequenceCount > l_maxStructuredBufferElementCount, "MotionSequence数がuint32_tで表現できる範囲を超えており、SkeletalAnimationModel用StructuredBufferSourceDataの作成に失敗しました。", false);

	// MotionごとにBone数と同じTrack要素を作るため、
	// 乗算前に要素数がuint32_tの範囲を超えないことを確認する
	if (!l_modelMotionSequenceList.empty())
	{
		FWK_ASSERT_RETURN_VALUE_IF(l_boneCount > l_maxStructuredBufferElementCount / l_motionSequenceCount, "BoneMotionTrackBufferの要素数がuint32_tで表現できる範囲を超えています。", false);
	}

	const auto& l_denseBoneMotionTrackCount = l_boneCount * l_motionSequenceCount;

	// 処理途中で失敗しても、出力先へ作成途中のデータを残さないように、
	// ローカル変数へすべて作成してから最後にMoveする
	StructuredBufferSourceData l_structuredBufferSourceData = {};

	l_structuredBufferSourceData.m_boneBufferElementList.reserve           (l_boneCount);
	l_structuredBufferSourceData.m_motionSequenceBufferElementList.reserve (l_motionSequenceCount);
	l_structuredBufferSourceData.m_boneMotionTrackBufferElementList.reserve(l_denseBoneMotionTrackCount);

	for (auto l_boneIndex = 0ULL; l_boneIndex < l_boneCount; ++l_boneIndex)
	{
		      auto& l_modelBone       = l_modelBoneList[l_boneIndex];
		const auto& l_parentBoneIndex = l_modelBone.m_parentBoneIndex;

		if (l_parentBoneIndex != SkeletalAnimationModelRecord::k_invalidBoneIndex)
		{
			FWK_ASSERT_RETURN_VALUE_IF(l_parentBoneIndex >= l_boneIndex, "ModelBoneListが親Boneから子Boneの順番で並んでいません。", false);
		}

		BoneBufferElement l_boneBufferElement = {};

		// Animation BlendではMatrix同士を直接補間せず、
		// Scale、Rotation、Translationを個別に補間する。
		// Trackを持たないBoneではBindPoseを使うため、ここでBindPose MatrixをSRTへ分解する
		FWK_ASSERT_RETURN_VALUE_IF(!l_modelBone.m_bindPoseLocalMatrix.Decompose(l_boneBufferElement.m_bindPoseLocalScale, l_boneBufferElement.m_bindPoseLocalRotation, l_boneBufferElement.m_bindPoseLocalTranslation), "BindPoseLocalMatrixをScale、Rotation、Translationへ分解できませんでした。", false);

		l_boneBufferElement.m_parentBoneIndex = l_parentBoneIndex;

		if (l_parentBoneIndex != SkeletalAnimationModelRecord::k_invalidBoneIndex)
		{
			const auto& l_parentBoneBufferElement = l_structuredBufferSourceData.m_boneBufferElementList[l_parentBoneIndex];

			l_boneBufferElement.m_hierarchyDepth = l_parentBoneBufferElement.m_hierarchyDepth + k_childHierarchyDepthOffset;
		}

		l_structuredBufferSourceData.m_maxBoneHierarchyDepth = std::max  (l_structuredBufferSourceData.m_maxBoneHierarchyDepth, l_boneBufferElement.m_hierarchyDepth);
		l_structuredBufferSourceData.m_boneBufferElementList.emplace_back(l_boneBufferElement);
	}

	// CPU側のMotionTrackは、Animationを持つBoneのTrackだけを格納した疎な配列になっている。
	// Bone IndexからTrackを直接取得できるように、一時的な参照配列を作る
	std::vector<const SkeletalAnimationModelRecord::ModelBoneMotionTrack*> l_tempModelBoneMotionTrackList = {};

	l_tempModelBoneMotionTrackList.resize(l_boneCount, nullptr);

	std::size_t l_totalKeyFrameCount = k_initialTotalKeyFrameCount;

	// KeyFrame Bufferの必要要素数を求めながら、
	// Motion、Track、KeyFrameの参照関係が正常であることを確認する
	for (const auto& l_modelMotionSequence : l_modelMotionSequenceList)
	{
		const auto& l_modelBoneMotionTrackList = l_modelMotionSequence.m_boneMotionTrackList;

		FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneMotionTrackList.empty(),                                                                      "MotionSequenceのBoneMotionTrackListが空です。",                       false);
		FWK_ASSERT_RETURN_VALUE_IF(l_modelBoneMotionTrackList.size() > l_boneCount,                                                         "MotionSequenceのBoneMotionTrack数がSkeletonのBone数を超えています。", false);
		FWK_ASSERT_RETURN_VALUE_IF(l_modelMotionSequence.m_durationSecond < SkeletalAnimationModelRecord::k_initialAnimationDurationSecond, "MotionSequenceの再生時間が0未満です。",                               false);
		FWK_ASSERT_RETURN_VALUE_IF(l_modelMotionSequence.m_frameRate <= MotionSequenceBufferElement::k_initialFrameRate,                    "MotionSequenceのFrameRateが0以下です。",                              false);

		// 前のMotionで設定したTrackの参照を残すと、
		// 現在のMotionに存在しないTrackまで誤って参照してしまう。
		// そのため、Motionの検証を始めるたびに一時参照配列を空へ戻す
		std::fill(l_tempModelBoneMotionTrackList.begin(), l_tempModelBoneMotionTrackList.end(), nullptr);

		for (const auto& l_modelBoneMotionTrack : l_modelBoneMotionTrackList)
		{
			const auto& l_boneIndex     = l_modelBoneMotionTrack.m_boneIndex;
			const auto& l_keyFrameList  = l_modelBoneMotionTrack.m_keyFrameList;
			const auto& l_keyFrameCount = l_keyFrameList.size();

			FWK_ASSERT_RETURN_VALUE_IF(l_boneIndex >= l_boneCount,                                                 "BoneMotionTrackがSkeletonの範囲外のBoneIndexを参照しています。",         false);
			FWK_ASSERT_RETURN_VALUE_IF(l_tempModelBoneMotionTrackList[l_boneIndex],                                "同一MotionSequence内に同じBoneIndexのBoneMotionTrackが複数存在します。", false);
			FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameList.empty(),                                                     "BoneMotionTrackのKeyFrameListが空です。",                                false);
			FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameCount > l_maxStructuredBufferElementCount,                        "BoneMotionTrackのKeyFrame数がuint32_tで表現できる範囲を超えています。",  false);
			FWK_ASSERT_RETURN_VALUE_IF(l_keyFrameCount > l_maxStructuredBufferElementCount - l_totalKeyFrameCount, "KeyFrameBufferの要素数がuint32_tで表現できる範囲を超えています。",       false);

			l_tempModelBoneMotionTrackList[l_boneIndex] = &l_modelBoneMotionTrack;
			
			// 総キーフレームカウントに加算する
			l_totalKeyFrameCount += l_keyFrameCount;

			for (auto l_keyFrameIndex = 0ULL; l_keyFrameIndex < l_keyFrameCount; ++l_keyFrameIndex)
			{
				const auto& l_modelKeyFrame = l_keyFrameList[l_keyFrameIndex];

				FWK_ASSERT_RETURN_VALUE_IF(l_modelKeyFrame.m_timeSecond < SkeletalAnimationModelRecord::k_initialAnimationTimeSecond, "KeyFrameのTimeSecondが0未満です。",                              false);
				FWK_ASSERT_RETURN_VALUE_IF(l_modelKeyFrame.m_timeSecond > l_modelMotionSequence.m_durationSecond,                     "KeyFrameのTimeSecondがMotionSequenceの再生時間を超えています。", false);

				if (l_keyFrameIndex != k_initialKeyFrameIndex)
				{
					const auto& l_previousModelKeyFrame = l_keyFrameList[l_keyFrameIndex - k_previousKeyFrameIndexOffset];

					FWK_ASSERT_RETURN_VALUE_IF(l_modelKeyFrame.m_timeSecond < l_previousModelKeyFrame.m_timeSecond, "BoneMotionTrackのKeyFrameが時間順に並んでいません。", false);
				}
			}
		}
	}

	// キーフレームを総数分容量を確保
	l_structuredBufferSourceData.m_keyFrameBufferElementList.reserve(l_totalKeyFrameCount);

	for (const auto& l_modelMotionSequence : l_modelMotionSequenceList)
	{
		MotionSequenceBufferElement l_motionSequenceBufferElement = {};

		l_motionSequenceBufferElement.m_durationSecond = l_modelMotionSequence.m_durationSecond;
		l_motionSequenceBufferElement.m_frameRate      = l_modelMotionSequence.m_frameRate;
		l_motionSequenceBufferElement.m_firstBoneMotionTrackIndex = static_cast<std::uint32_t>(l_structuredBufferSourceData.m_boneMotionTrackBufferElementList.size());

		l_structuredBufferSourceData.m_motionSequenceBufferElementList.emplace_back(l_motionSequenceBufferElement);

		std::fill(l_tempModelBoneMotionTrackList.begin(), l_tempModelBoneMotionTrackList.end(), nullptr);

		for (const auto& l_modelBoneMotionTrack : l_modelMotionSequence.m_boneMotionTrackList)
		{
			l_tempModelBoneMotionTrackList[l_modelBoneMotionTrack.m_boneIndex] = &l_modelBoneMotionTrack;
		}

		// MotionごとにBone数と同じTrack要素を作る。
		// これによりCompute Shaderでは、
		// firstBoneMotionTrackIndex + boneIndexだけでTrackを直接参照できる
		for (auto l_boneIndex = 0ULL; l_boneIndex < l_boneCount; ++l_boneIndex)
		{
			BoneMotionTrackBufferElement l_boneMotionTrackBufferElement = {};

			const auto* l_modelBoneMotionTrack = l_tempModelBoneMotionTrackList[l_boneIndex];

			// Trackを持たないBoneはKeyFrameCountを0のままにする。
			// Compute ShaderではKeyFrameCountが0の場合にBindPoseを使用する
			if (l_modelBoneMotionTrack)
			{
				const auto& l_modelKeyFrameList = l_modelBoneMotionTrack->m_keyFrameList;

				l_boneMotionTrackBufferElement.m_firstKeyFrameIndex = static_cast<std::uint32_t>(l_structuredBufferSourceData.m_keyFrameBufferElementList.size());
				l_boneMotionTrackBufferElement.m_keyFrameCount      = static_cast<std::uint32_t>(l_modelKeyFrameList.size());

				for (const auto& l_modelKeyFrame : l_modelKeyFrameList)
				{
					KeyFrameBufferElement l_keyFrameBufferElement = {};

					l_keyFrameBufferElement.m_scale       = l_modelKeyFrame.m_scale;
					l_keyFrameBufferElement.m_rotation    = l_modelKeyFrame.m_rotation;
					l_keyFrameBufferElement.m_translation = l_modelKeyFrame.m_translation;
					l_keyFrameBufferElement.m_timeSecond  = l_modelKeyFrame.m_timeSecond;

					l_structuredBufferSourceData.m_keyFrameBufferElementList.emplace_back(l_keyFrameBufferElement);
				}
			}

			l_structuredBufferSourceData.m_boneMotionTrackBufferElementList.emplace_back(l_boneMotionTrackBufferElement);
		}
	}

	a_structuredBufferSourceData = std::move(l_structuredBufferSourceData);

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