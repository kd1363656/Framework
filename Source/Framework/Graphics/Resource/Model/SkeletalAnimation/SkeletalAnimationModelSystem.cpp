#include "SkeletalAnimationModelSystem.h"

bool FWK::Graphics::SkeletalAnimationModelSystem::LoadSkeletalAnimationModelAsset(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	FWK_ASSERT_RETURN_VALUE_IF(!Utility::CanLoadFilePath(a_filePath, FBXModelLoaderBase::k_lowerFBXExtension), "SkeletalAnimationModelのFBXファイルが存在しません。", false);

	// 有効な.assetが存在する場合は、FBXの解析を行わず直接読み込み
	if (m_binaryConverter.LoadAsset(a_filePath, a_skeletalAnimationModelRecord)) 
	{
		// テクスチャの読み込み(バイナリーファイルになっていなければバイナリーファイル化する)
		m_materialRuntimeTextureBuilder.BuildMaterialRuntimeTextures(a_filePath, a_skeletalAnimationModelRecord);

		return true; 
	}

	// .assetを使用できない場合はFBXからModelDataを抽出する
	FWK_ASSERT_RETURN_VALUE_IF(!m_loader.LoadSkeletalAnimationModelFile(a_filePath, a_skeletalAnimationModelRecord), "SkeletalAnimationModelのFBX読み込みに失敗しました。", false);

	// FBXから読み込んだModelDataを最適化し、
	// Meshletを生成して.assetへ保存する
	FWK_ASSERT_RETURN_VALUE_IF(!BuildSkeletalAnimationModelAssetData(a_filePath, a_skeletalAnimationModelRecord), "SkeletalAnimationModelのAssetData作成に失敗しました。", false);

	m_materialRuntimeTextureBuilder.BuildMaterialRuntimeTextures(a_filePath, a_skeletalAnimationModelRecord);

	return true;
}

bool FWK::Graphics::SkeletalAnimationModelSystem::BuildSkeletalAnimationModelAssetData(const std::filesystem::path& a_filePath, SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord)
{
	// meshoptimizerを使用して頂点とIndexをGPU向けに最適化する
	FWK_ASSERT_RETURN_VALUE_IF(!m_meshOptimizer.OptimizeModelRecord(a_skeletalAnimationModelRecord), "SkeletalAnimationModelMeshの最適化に失敗しました。", false);

	// 最適化済みの頂点とIndexからMeshletDataを作成する
	FWK_ASSERT_RETURN_VALUE_IF(!m_meshletBuilder.BuildModelRecordMeshletData(a_skeletalAnimationModelRecord), "SkeletalAnimationModelMeshletDataの作成に失敗しました。", false);

	// 完成したAssetDataを保存し、
	// 次回以降はFBX解析・最適化・Meshlet生成を省略できるようにする
	FWK_ASSERT_RETURN_VALUE_IF(!m_binaryConverter.SaveAsset(a_filePath, a_skeletalAnimationModelRecord), "SkeletalAnimationModelAssetの保存に失敗しました。", false);

	return true;
}