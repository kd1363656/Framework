#include "SkeletalAnimationModelBinaryConverter.h"

bool FWK::Converter::SkeletalAnimationModelBinaryConverter::LoadAsset(const std::filesystem::path& a_filePath, Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	auto& l_modelData = a_skeletalAnimationModelRecord.GetMutableREFAnimationData();

	// 途中まで読み込んだ古いデータが残らないように、最初に空にする
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

	auto l_memoyReadOffset = k_initialMemoryReadOffset;

	ModelBinaryHeader l_modelBinaryHeader = {};

	// ファイル先頭の全体Headerを読み込む
	if (!TryReadSingleBinaryData(l_modelBinaryHeader, l_memoyReadOffset))
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// SkeltalAnimationMode用.assertではないなら読み込まない
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

	// Boneが0このSkeltalAnimationModelは成立しない
	if (l_modelBinaryHeader.m_boneCount == k_emptyBoneCount)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	// BoneList読み込み
	l_modelData.m_boneList.resize(l_modelBinaryHeader.m_boneCount);

	for (auto& l_modelBone : l_modelData.m_boneList)
	{
		ModelBoneBinaryHeader l_modelBoneBinaryHeader = {};

		// Bone単位Headerを読み込む
		if (TryReadSingleBinaryData(l_modelBoneBinaryHeader, l_memoyReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// Bone名を読み込む
		if (!TryReadWStringBinaryData(l_modelBoneBinaryHeader.m_boneNameSize, l_modelBone.m_boneName, l_memoyReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		l_modelBone.m_parentBoneIndex = l_modelBoneBinaryHeader.m_parentBoneIndex;

		// BindPose時点のLocalMatrix
		if (!TryReadSingleBinaryData(l_modelBone.m_bindPoseLocalMatrix, l_memoyReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		// SkinningMatrix作成時に使うInverseBindPoseMatrix
		if (!TryReadSingleBinaryData(l_modelBone.m_inverseBindPoseMatrix, l_memoyReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}
	}

	// MotionSequecenList読み込み
	for (auto& l_modelMotionSequence : l_modelData.m_motionSequenceList)
	{
		ModelMotionSequenceBinaryHeader l_modelMotionSequenceBinaryHeader = {};

		// MotionSequence単位Headerを読み込む
		if (!TryReadSingleBinaryData(l_modelMotionSequenceBinaryHeader, l_memoyReadOffset))
		{
			FailLoadAsset(l_modelData);

			return false;
		}

		l_modelMotionSequence.m_durationSecond = l_modelMotionSequenceBinaryHeader.m_durationSecond;
		l_modelMotionSequence.m_motionName     = l_modelMotionSequenceBinaryHeader.m_frameRate;

		// Motion名を読み込む
		if (!TryReadWStringBinaryData(l_modelMotionSequenceBinaryHeader.m_motionNameSize, l_modelMotionSequence.m_motionName, l_memoyReadOffset))
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
			if (!TryReadSingleBinaryData(l_modelBoneMotionTrackBinaryHeader, l_memoyReadOffset))
			{
				FailLoadAsset(l_modelData);

				return false;
			}

			l_modelBoneMotionTrack.m_boneIndex = l_modelBoneMotionTrackBinaryHeader.m_boneIndex;

			// KeyFrame配列を読み込む
			// KeyFrameは固定長structなので、まとめて読み込める
			if (TryReadBinaryDataList(l_modelBoneMotionTrackBinaryHeader.m_keyFrameCount, l_modelBoneMotionTrack.m_keyFrameList, l_memoyReadOffset))
			{
				FailLoadAsset(l_modelData);

				return false;
			}
		}
	}

	// 最後まで読み込んだ位置がHeaderのファイルサイズと一致しないなら、
	// 読み込み純化ファイルサイズ計算が間違っている
	if (l_memoyReadOffset != l_modelBinaryHeader.m_fileSize)
	{
		FailLoadAsset(l_modelData);

		return false;
	}

	DestroyMemoryMappedFile();

	return true;
}
bool FWK::Converter::SkeletalAnimationModelBinaryConverter::SaveAsset(const std::filesystem::path& a_filePath, Graphics::SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	return false;
}

bool FWK::Converter::SkeletalAnimationModelBinaryConverter::CanLoadAsset(const std::filesystem::path& a_filePath) const
{
	return false;
}

void FWK::Converter::SkeletalAnimationModelBinaryConverter::FailLoadAsset(Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData)
{

}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData, const std::uint64_t& a_fileSize) const
{
	return ModelBinaryHeader();
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelBoneBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelBoneBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelBone& a_modelBone) const
{
	return ModelBoneBinaryHeader();
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelMotionSequenceBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateMotionSequenceBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelMotionSequence& a_modelMotionSequence) const
{
	return ModelMotionSequenceBinaryHeader();
}

FWK::Converter::SkeletalAnimationModelBinaryConverter::ModelBoneMotionTrackBinaryHeader FWK::Converter::SkeletalAnimationModelBinaryConverter::CreateModelBoneMotionTrackBinaryHeader(const Graphics::SkeletalAnimationModelRecord::ModelBoneMotionTrack& a_modelBoneMotionTrack) const
{
	return ModelBoneMotionTrackBinaryHeader();
}

std::uint64_t FWK::Converter::SkeletalAnimationModelBinaryConverter::CalculateAssetFileSize(const Graphics::SkeletalAnimationModelRecord::ModelData& a_modelData) const
{
	return std::uint64_t();
}
