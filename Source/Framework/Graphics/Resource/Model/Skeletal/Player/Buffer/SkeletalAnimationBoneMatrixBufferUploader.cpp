#include "SkeletalAnimationBoneMatrixBufferUploader.h"

FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader::SkeletalAnimationBoneMatrixBufferUploader() :
	DynamicBufferUploaderBase(sizeof(TypeAlias::Math::Matrix), k_keepWritePosition)
{}
FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader::~SkeletalAnimationBoneMatrixBufferUploader() = default;

FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader::SkeletalAnimationBoneMatrixBufferUploader(SkeletalAnimationBoneMatrixBufferUploader&& a_other) noexcept : 
	DynamicBufferUploaderBase(std::move(a_other))
{}

bool FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader::Create(const Device& a_device)
{
	FWK_ASSERT_RETURN_VALUE_IF(!CreateUploadBuffer(a_device, sizeof(TypeAlias::Math::Matrix)), "BoneMatrix用UploadBufferの作成に失敗しました。", false);

	return true;
}

bool FWK::Graphics::SkeletalAnimationBoneMatrixBufferUploader::Write(const std::vector<TypeAlias::Math::Matrix>& a_boneMatrixList)
{
	// vectorが保持するMatrixを複製せず、
	// 連続した範囲としてUploadBufferへ書き込む。
	// 固定位置Uploaderなので、
	// 毎回UploadBufferの先頭から上書きする。
	const std::span<const TypeAlias::Math::Matrix> l_boneMatrixRange = { a_boneMatrixList };

	return WriteElementRange(l_boneMatrixRange) != k_invalidGPUVirtualAddress;
}
