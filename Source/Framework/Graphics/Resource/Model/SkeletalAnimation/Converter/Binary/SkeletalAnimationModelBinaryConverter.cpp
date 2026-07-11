#include "SkeletalAnimationModelBinaryConverter.h"

bool FWK::Converter::SkeletalAnimationModelBinaryConverter::LoadAsset(const std::filesystem::path& a_filePath, Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFModelData();

	// 途中まで読み込んだ古いデータが残らないように、最初に空にする
	l_modelData.m_modelMeshList.clear     ();
	l_modelData.m_boneList.clear          ();
	l_modelData.m_motionSequenceList.clear();

	// .assetが存在しない、FBXより古い、元FBXが存在しない場合はfalse
	// 呼び出し側でFBXから再読み込みして、.assetを再生成する
	if (!CanLoadAsset(a_filePath)) { return false; }

	// FBXと同名で拡張子だけ.assetにしたパスを作成し、読み込み用MemoryMappedFileを開く
	if (const auto& l_modelAssetFilePath = CreateAssetFilePath(a_filePath);
		!CreateReadMemoryMappedFile(l_modelAssetFilePath))
	{
		return false;
	}

	auto l_memoryReadOffset = k_initialMemoryReadOffset;

	ModelBinaryHeader l_modelBinaryHeader = {};

	// ファイル先頭の全体Headerを読み込む
	if (!TryReadSingleBinaryData(l_modelBinaryHeader, l_memoryReadOffset))
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// Model用.assertではないなら読み込まない
	if (l_modelBinaryHeader.m_assetTypeID != k_modelAssetTypeID)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// 保存形式が古いなら読み込まない
	// 呼び出し側でFBXから再生成する
	if (l_modelBinaryHeader.m_version != k_modelAssetVersion)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// Header内のファイルサイズと実際のファイルサイズが違うなら壊れている可能性がある
	if (l_modelBinaryHeader.m_fileSize != GetREFMappedDataSize())
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// Meshが0個のSkeletalAnimationModelは描画できない
	if (l_modelBinaryHeader.m_modelMeshCount == Constant::k_emptyModelMeshCount)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// Boneが0このSkeltalAnimationModelは成立しない
	if (l_modelBinaryHeader.m_boneCount == k_emptyBoneCount)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// ModelMesh読み込み
	l_modelData.m_modelMeshList.resize(l_modelBinaryHeader.m_modelMeshCount);

	for (auto& l_modelMesh : l_modelData.m_modelMeshList)
	{
		ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

		if (!TryReadSingleBinaryData(l_modelMeshBinaryHeader, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// 頂点情報を読み込む
		if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_vertexCount, l_modelMesh.m_modelVertexList, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// インデックス情報を読み込む
		if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_indexCount, l_modelMesh.m_indexList, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		auto& l_modelMaterialAssetData = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;

		// ベースカラー倍率の読み込み
		if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_baseColorFactor, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// ラフネス倍率の読み込み
		if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_roughnessFactor, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// メタリック倍率の読み込み
		if (!TryReadSingleBinaryData(l_modelMaterialAssetData.m_metallicFactor, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// ベースカラーテクスチャファイルパスの読み込み
		if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_baseColorTextureFilePathSize, l_modelMaterialAssetData.m_baseColorTextureFilePath, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// 法線テクスチャファイルパスの読み込み
		if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_normalTextureFilePathSize, l_modelMaterialAssetData.m_normalTextureFilePath, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// ラフネステクスチャファイルパスの読み込み
		if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_roughnessTextureFilePathSize, l_modelMaterialAssetData.m_roughnessTextureFilePath, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// メタリックテクスチャファイルパスの読み込み
		if (!TryReadWStringBinaryData(l_modelMeshBinaryHeader.m_metallicTextureFilePathSize, l_modelMaterialAssetData.m_metallicTextureFilePath, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// Texture本体は.assetに保存しない
		l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData = {};

		auto& l_modelMeshletData = l_modelMesh.m_modelMeshletData;

		// メッシュレットリストの読み込み
		if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletCount, l_modelMeshletData.m_meshletList, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// メッシュレットリストの読み込み
		if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// プリミティブインデックスリストの読み込み
		if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_primitiveIndexCount, l_modelMeshletData.m_primitiveIndexList, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// メッシュレットバウンドリストの読み込み
		if (!TryReadBinaryDataList(l_modelMeshBinaryHeader.m_meshletBoundsCount, l_modelMeshletData.m_meshletBoundsList, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}
	}

	// BoneList読み込み
	l_modelData.m_boneList.resize(l_modelBinaryHeader.m_boneCount);

	for (auto& l_modelBone : l_modelData.m_boneList)
	{
		ModelBoneBinaryHeader l_modelBoneBinaryHeader = {};

		// Bone単位Headerを読み込む
		if (!TryReadSingleBinaryData(l_modelBoneBinaryHeader, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// Bone名を読み込む
		if (!TryReadWStringBinaryData(l_modelBoneBinaryHeader.m_boneNameSize, l_modelBone.m_boneName, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		l_modelBone.m_parentBoneIndex = l_modelBoneBinaryHeader.m_parentBoneIndex;

		// BindPose時点のLocalMatrix
		if (!TryReadSingleBinaryData(l_modelBone.m_bindPoseLocalMatrix, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// SkinningMatrix作成時に使うInverseBindPoseMatrix
		if (!TryReadSingleBinaryData(l_modelBone.m_inverseBindPoseMatrix, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}
	}

	// MotionSequenceList読み込み
	l_modelData.m_motionSequenceList.resize(l_modelBinaryHeader.m_motionSequenceCount);

	for (auto& l_modelMotionSequence : l_modelData.m_motionSequenceList)
	{
		ModelMotionSequenceBinaryHeader l_modelMotionSequenceBinaryHeader = {};

		// MotionSequence単位Headerを読み込む
		if (!TryReadSingleBinaryData(l_modelMotionSequenceBinaryHeader, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		l_modelMotionSequence.m_durationSecond = l_modelMotionSequenceBinaryHeader.m_durationSecond;
		l_modelMotionSequence.m_frameRate      = l_modelMotionSequenceBinaryHeader.m_frameRate;

		// Motion名を読み込む
		if (!TryReadWStringBinaryData(l_modelMotionSequenceBinaryHeader.m_motionNameSize, l_modelMotionSequence.m_motionName, l_memoryReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// BoneMotionTrackList読み込み
		l_modelMotionSequence.m_boneMotionTrackList.resize(l_modelMotionSequenceBinaryHeader.m_boneMotionTrackCount);

		for (auto& l_modelBoneMotionTrack : l_modelMotionSequence.m_boneMotionTrackList)
		{
			ModelBoneMotionTrackBinaryHeader l_modelBoneMotionTrackBinaryHeader = {};

			// BoneMotionTrack単位Headerを読み込む
			if (!TryReadSingleBinaryData(l_modelBoneMotionTrackBinaryHeader, l_memoryReadOffset))
			{
				FailLoadAsset(l_modelData);

				return false;
			}

			l_modelBoneMotionTrack.m_boneIndex = l_modelBoneMotionTrackBinaryHeader.m_boneIndex;

			// KeyFrame配列を読み込む
			// KeyFrameは固定長structなので、まとめて読み込める
			if (!TryReadBinaryDataList(l_modelBoneMotionTrackBinaryHeader.m_keyFrameCount, l_modelBoneMotionTrack.m_keyFrameList, l_memoryReadOffset))
			{
				FailLoadAsset(l_modelData);

				return false;
			}
		}
	}

	// 最後まで読み込んだ位置がHeaderのファイルサイズと一致しないなら、
	// 読み込み純化ファイルサイズ計算が間違っている
	if (l_memoryReadOffset != l_modelBinaryHeader.m_fileSize)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	DestroyMemoryMappedFile();

	return true;
}
bool FWK::Converter::SkeletalAnimationModelBinaryConverter::SaveAsset(const std::filesystem::path& a_filePath, const Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	const auto& l_modelData = a_skeletalAnimationModelRecord.GetREFModelData();

	// SkeletalAnimationModelAssetはFBXから生成する想定
	// 元FBXが存在しない場合、何から生成された.assetか判断できないので保存しない
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, Graphics::FBXModelLoaderBase::k_lowerFBXExtension), "Assetの元になるFBXファイルが無効となっており、バイナリーファイルの保存に失敗しました。", false);

	// FBXと同じ場所・同じ名前で拡張子だけ.assetにした保存先を作る
	const auto& l_modelAssetFilePath = CreateAssetFilePath(a_filePath);

	// 保存に必要なファイルサイズを先に計算する
	const auto& l_modelAssetFileSize = CalculateAssetFileSize(l_modelData);

	// Boneがないなど、保存できるデータがない場合は.asset化しない
	FWK_ASSERT_RETURN_VALUE_IF(l_modelAssetFileSize == k_emptyAssetFileSize, "Assetへ保存するModelDataが空のため、バイナリーファイルの保存に失敗しました。", false);

	// 書き込み用MemoryMappedFileを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!CreateWriteMemoryMappedFile(l_modelAssetFilePath,l_modelAssetFileSize), "Assetの書き込み用MemoryMappedFile作成に失敗しました。", false);

	auto l_memoryWriteOffset = k_initialMemoryWriteOffset;

	// 全体Headerを書き込む
	const auto& l_modelBinaryHeader = CreateModelBinaryHeader(l_modelData, l_modelAssetFileSize);

	WriteBinaryData(k_singleBinaryElementCount, &l_modelBinaryHeader, l_memoryWriteOffset);

	for (const auto& l_modelMesh : l_modelData.m_modelMeshList)	
	{
		const auto& l_modelMeshBinaryHeader = CreateModelMeshBinaryHeader(l_modelMesh);

		WriteBinaryData(k_singleBinaryElementCount,            &l_modelMeshBinaryHeader,              l_memoryWriteOffset);
		WriteBinaryData(l_modelMeshBinaryHeader.m_vertexCount,  l_modelMesh.m_modelVertexList.data(), l_memoryWriteOffset);
		WriteBinaryData(l_modelMeshBinaryHeader.m_indexCount,   l_modelMesh.m_indexList.data(),       l_memoryWriteOffset);

		const auto& l_modelMaterialAssetData = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;

		WriteBinaryData(k_singleBinaryElementCount, &l_modelMaterialAssetData.m_baseColorFactor, l_memoryWriteOffset);
		WriteBinaryData(k_singleBinaryElementCount, &l_modelMaterialAssetData.m_roughnessFactor, l_memoryWriteOffset);
		WriteBinaryData(k_singleBinaryElementCount, &l_modelMaterialAssetData.m_metallicFactor,  l_memoryWriteOffset);

		WriteWStringBinaryData(l_modelMaterialAssetData.m_baseColorTextureFilePath, l_memoryWriteOffset);
		WriteWStringBinaryData(l_modelMaterialAssetData.m_normalTextureFilePath,    l_memoryWriteOffset);
		WriteWStringBinaryData(l_modelMaterialAssetData.m_roughnessTextureFilePath, l_memoryWriteOffset);
		WriteWStringBinaryData(l_modelMaterialAssetData.m_metallicTextureFilePath,  l_memoryWriteOffset);

		const auto& l_modelMeshletData = l_modelMesh.m_modelMeshletData;

		WriteBinaryData(l_modelMeshBinaryHeader.m_meshletCount,           l_modelMeshletData.m_meshletList.data          (), l_memoryWriteOffset);
		WriteBinaryData(l_modelMeshBinaryHeader.m_uniqueVertexIndexCount, l_modelMeshletData.m_uniqueVertexIndexList.data(), l_memoryWriteOffset);
		WriteBinaryData(l_modelMeshBinaryHeader.m_primitiveIndexCount,    l_modelMeshletData.m_primitiveIndexList.data   (), l_memoryWriteOffset);
		WriteBinaryData(l_modelMeshBinaryHeader.m_meshletBoundsCount,     l_modelMeshletData.m_meshletBoundsList.data    (), l_memoryWriteOffset);


	}

	// BoneListを書き込む
	for (const auto& l_modelBone : l_modelData.m_boneList)
	{
		const auto& l_modelBoneBinaryHeader = CreateModelBoneBinaryHeader(l_modelBone);

		// Bone単位Header
		WriteBinaryData(k_singleBinaryElementCount, &l_modelBoneBinaryHeader, l_memoryWriteOffset);

		// Bone名
		WriteWStringBinaryData(l_modelBone.m_boneName, l_memoryWriteOffset);

		// BindPoseLocalMatrix
		WriteBinaryData(k_singleBinaryElementCount, &l_modelBone.m_bindPoseLocalMatrix, l_memoryWriteOffset);

		// InverseBindPoseMatrix
		WriteBinaryData(k_singleBinaryElementCount, &l_modelBone.m_inverseBindPoseMatrix, l_memoryWriteOffset);
	}

	// MotionSequenceListを書き込む
	for (const auto& l_modelMotionSequence : l_modelData.m_motionSequenceList)
	{
		const auto& l_modelMotionSequenceBinaryHeader = CreateMotionSequenceBinaryHeader(l_modelMotionSequence);

		// MotionSequence単位Header
		WriteBinaryData(k_singleBinaryElementCount, &l_modelMotionSequenceBinaryHeader, l_memoryWriteOffset);

		// Motion名
		WriteWStringBinaryData(l_modelMotionSequence.m_motionName, l_memoryWriteOffset);

		// BoneMotionTrackList
		for (const auto& l_modelBoneMotionTrack : l_modelMotionSequence.m_boneMotionTrackList)
		{
			const auto& l_modelBoneMotionTrackBinaryHeader = CreateModelBoneMotionTrackBinaryHeader(l_modelBoneMotionTrack);

			// BoneMotionTrack単位Header
			WriteBinaryData(k_singleBinaryElementCount, &l_modelBoneMotionTrackBinaryHeader, l_memoryWriteOffset);

			// KeyFrame配列
			WriteBinaryData(l_modelBoneMotionTrackBinaryHeader.m_keyFrameCount, l_modelBoneMotionTrack.m_keyFrameList.data(), l_memoryWriteOffset);
		}
	}

	// 計算したファイルサイズと実際に書き込んだサイズが一致するか確認する
	if (l_memoryWriteOffset != l_modelAssetFileSize)
	{
		DestroyMemoryMappedFile();

		FWK_ASSERT_RETURN_VALUE("Assetの書き込みサイズが計算したファイルサイズと一致せず、バイナリーファイルの保存に失敗しました。", false);
	}

	DestroyMemoryMappedFile();

	return true;
}

bool FWK::Converter::SkeletalAnimationModelBinaryConverter::CanLoadAsset(const std::filesystem::path& a_filePath) const
{
	// 元となるFBXが存在しない場合は、.assetの正当性を判断できないので読み込まない
	if (!Utility::CanLoadFilePath(a_filePath, Graphics::FBXModelLoaderBase::k_lowerFBXExtension)) { return false; }

	const auto& l_modelAssetFilePath = CreateAssetFilePath(a_filePath);

	// .assetが存在しないなら、FBXから読み込んで生成する
	if (!Utility::CanLoadFilePath(l_modelAssetFilePath, BinaryFileConverterBase::k_lowerAssetExtension)) { return false; }

	// FBXが.assetより新しいなら、古い.assetは使わない
	if (IsUpdatedSourceFile(a_filePath, l_modelAssetFilePath)) { return false; }

	return true;
}

void FWK::Converter::SkeletalAnimationModelBinaryConverter::FailLoadAsset(Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData)
{
	// 中途半端に読み込んだデータが残らないように空にする
	a_modelData.m_modelMeshList.clear     ();
	a_modelData.m_boneList.clear          ();
	a_modelData.m_motionSequenceList.clear();

	// 読み込み途中で失敗した場合も、MemoryMappedFileは必ず閉じる
	DestroyMemoryMappedFile();
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData, const std::uint64_t& a_fileSize) const
{
	ModelBinaryHeader l_modelBinaryHeader = {};

	l_modelBinaryHeader.m_fileSize            = a_fileSize;
	l_modelBinaryHeader.m_version             = k_modelAssetVersion;
	l_modelBinaryHeader.m_assetTypeID         = k_modelAssetTypeID;
	l_modelBinaryHeader.m_modelMeshCount      = a_modelData.m_modelMeshList.size     ();
	l_modelBinaryHeader.m_boneCount           = a_modelData.m_boneList.size          ();
	l_modelBinaryHeader.m_motionSequenceCount = a_modelData.m_motionSequenceList.size();

	return l_modelBinaryHeader;
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelMeshBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelMeshBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelMesh& a_modelMesh) const
{
	ModelMeshBinaryHeader l_modelMeshBinaryHeader = {};

	const auto& l_modelMaterialAssetData = a_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
	const auto& l_modelMeshletData       = a_modelMesh.m_modelMeshletData;

	l_modelMeshBinaryHeader.m_vertexCount = a_modelMesh.m_modelVertexList.size();
	l_modelMeshBinaryHeader.m_indexCount  = a_modelMesh.m_indexList.size      ();

	l_modelMeshBinaryHeader.m_baseColorTextureFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_baseColorTextureFilePath);
	l_modelMeshBinaryHeader.m_normalTextureFilePathSize    = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_normalTextureFilePath);
	l_modelMeshBinaryHeader.m_roughnessTextureFilePathSize = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_roughnessTextureFilePath);
	l_modelMeshBinaryHeader.m_metallicTextureFilePathSize  = CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_metallicTextureFilePath);

	l_modelMeshBinaryHeader.m_meshletCount           = l_modelMeshletData.m_meshletList.size          ();
	l_modelMeshBinaryHeader.m_uniqueVertexIndexCount = l_modelMeshletData.m_uniqueVertexIndexList.size();
	l_modelMeshBinaryHeader.m_primitiveIndexCount    = l_modelMeshletData.m_primitiveIndexList.size   ();
	l_modelMeshBinaryHeader.m_meshletBoundsCount     = l_modelMeshletData.m_meshletBoundsList.size    ();

	return l_modelMeshBinaryHeader;
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelBoneBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelBoneBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelBone& a_modelBone) const
{
	ModelBoneBinaryHeader l_modelBoneBinaryHeader = {};

	l_modelBoneBinaryHeader.m_boneNameSize    = CalculateWStringBinaryFileSize(a_modelBone.m_boneName);
	l_modelBoneBinaryHeader.m_parentBoneIndex = a_modelBone.m_parentBoneIndex;

	return l_modelBoneBinaryHeader;
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelMotionSequenceBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateMotionSequenceBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelMotionSequence& a_modelMotionSequence) const
{
	ModelMotionSequenceBinaryHeader l_modelMotionSequenceBinaryHeader = {};

	l_modelMotionSequenceBinaryHeader.m_durationSecond       = a_modelMotionSequence.m_durationSecond;
	l_modelMotionSequenceBinaryHeader.m_frameRate            = a_modelMotionSequence.m_frameRate;
	l_modelMotionSequenceBinaryHeader.m_motionNameSize       = CalculateWStringBinaryFileSize                  (a_modelMotionSequence.m_motionName);
	l_modelMotionSequenceBinaryHeader.m_boneMotionTrackCount = a_modelMotionSequence.m_boneMotionTrackList.size();

	return l_modelMotionSequenceBinaryHeader;
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelBoneMotionTrackBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelBoneMotionTrackBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelBoneMotionTrack& a_modelBoneMotionTrack) const
{
	ModelBoneMotionTrackBinaryHeader l_modelMotionTrackBinaryHeader = {};

	l_modelMotionTrackBinaryHeader.m_keyFrameCount = a_modelBoneMotionTrack.m_keyFrameList.size();
	l_modelMotionTrackBinaryHeader.m_boneIndex     = a_modelBoneMotionTrack.m_boneIndex;

	return l_modelMotionTrackBinaryHeader;
}

std::uint64_t FWK::Converter::SkeletalAnimationModelBinaryConverter::CalculateAssetFileSize(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData) const
{
	// Meshが一つもないModelDataは.asset化しない
	if (a_modelData.m_modelMeshList.empty()) { return k_emptyAssetFileSize; }

	// Boneが一つもないModelDataは,asset化しない
	if (a_modelData.m_boneList.empty()) { return k_emptyAssetFileSize; }

	// ファイル先頭に置く全体Header
	auto l_modelAssetFileSize = CalculateBinaryDataSize<ModelBinaryHeader>(k_singleBinaryElementCount);

	for (const auto& l_modelMesh : a_modelData.m_modelMeshList)
	{
		const auto& l_modelMaterialAssetData = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
		const auto& l_modelMeshletData       = l_modelMesh.m_modelMeshletData;
	
		l_modelAssetFileSize += CalculateBinaryDataSize<ModelMeshBinaryHeader>(k_singleBinaryElementCount);
	
		l_modelAssetFileSize += CalculateBinaryDataSize<Graphics::SkeletalAnimationModelRecord::ModelVertex>(l_modelMesh.m_modelVertexList.size());
	
		l_modelAssetFileSize += CalculateBinaryDataSize<std::uint32_t>(l_modelMesh.m_indexList.size());
	
		l_modelAssetFileSize += CalculateBinaryDataSize<TypeAlias::Math::Color>(k_singleBinaryElementCount);
		l_modelAssetFileSize += CalculateBinaryDataSize<float>                 (k_singleBinaryElementCount);
		l_modelAssetFileSize += CalculateBinaryDataSize<float>                 (k_singleBinaryElementCount);
	
		l_modelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_baseColorTextureFilePath);
		l_modelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_normalTextureFilePath);
		l_modelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_roughnessTextureFilePath);
		l_modelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMaterialAssetData.m_metallicTextureFilePath);
	
		l_modelAssetFileSize += CalculateBinaryDataSize<Struct::ModelMeshlet>(l_modelMeshletData.m_meshletList.size());
	
		l_modelAssetFileSize += CalculateBinaryDataSize<std::uint32_t>(l_modelMeshletData.m_uniqueVertexIndexList.size());
	
		l_modelAssetFileSize += CalculateBinaryDataSize<std::uint32_t>(l_modelMeshletData.m_primitiveIndexList.size());
	
		l_modelAssetFileSize += CalculateBinaryDataSize<Struct::ModelMeshletBounds>(l_modelMeshletData.m_meshletBoundsList.size());
	}

	for (const auto& l_modelBone : a_modelData.m_boneList)
	{
		// Bone単位Header
		l_modelAssetFileSize += CalculateBinaryDataSize<ModelBoneBinaryHeader>(k_singleBinaryElementCount);

		// Bone名
		l_modelAssetFileSize += CalculateWStringBinaryFileSize(l_modelBone.m_boneName);

		// BindPoseLocalMatrix
		l_modelAssetFileSize += CalculateBinaryDataSize<TypeAlias::Math::Matrix>(k_singleBinaryElementCount);

		// InverseBindPoseMatrix
		l_modelAssetFileSize += CalculateBinaryDataSize<TypeAlias::Math::Matrix>(k_singleBinaryElementCount);
	}

	for (const auto& l_modelMotionSequence : a_modelData.m_motionSequenceList)
	{
		// MotionSequence単位Header
		l_modelAssetFileSize += CalculateBinaryDataSize<ModelMotionSequenceBinaryHeader>(k_singleBinaryElementCount);

		// Motion名
		l_modelAssetFileSize += CalculateWStringBinaryFileSize(l_modelMotionSequence.m_motionName);

		for (const auto& l_modelBoneMotionTrack : l_modelMotionSequence.m_boneMotionTrackList)
		{
			// BoneMotionTrack単位Header
			l_modelAssetFileSize += CalculateBinaryDataSize<ModelBoneMotionTrackBinaryHeader>(k_singleBinaryElementCount);

			// KeyFrame配列
			l_modelAssetFileSize += CalculateBinaryDataSize<Graphics::SkeletalAnimationModelRecord::ModelKeyFrame>(l_modelBoneMotionTrack.m_keyFrameList.size());
		}
	}

	return l_modelAssetFileSize;
}
