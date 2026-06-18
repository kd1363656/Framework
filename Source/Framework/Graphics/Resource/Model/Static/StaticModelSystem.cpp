#include "StaticModelSystem.h"

FWK::Struct::StaticModelResult FWK::Graphics::StaticModelSystem::LoadStaticModelForBatchUpload(const Device&				           a_device, 
																							   const GPUMemoryAllocator&           a_gpuMemoryAllocator, 
																							   const std::filesystem::path&        a_filePath, 
																									 TypeAlias::SRVDescriptorPool& a_srvDescriptorPool)
{
	Struct::StaticModelResult l_staticModelLoadResult = {};

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension), "StaticModelのFBXファイルが存在しません。", l_staticModelLoadResult);

	auto l_staticModelRecord = std::make_shared<Graphics::StaticModelRecord>();

	// 読み込み時間計測開始
	const Utility::Stopwatch& l_assetLoadStopwatch = {};

	// .assetが存在していて、FBXより更新が古くなければ.assetを優先して読み込む
	if (!m_binaryConverter.LoadStaticModelAsset(a_filePath, *l_staticModelRecord))
	{
		// .assetが読み込めなければテクスチャをロードする、失敗したらassert
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_loader.LoadStaticModelFile(a_filePath, *l_staticModelRecord), "StaticModel読み込みに失敗したため、バッチモデル登録に失敗しました。。", {});

		// 読み込んだテクスチャのデータを保存、次回以降はバイナリーファイルで読み込めるようにする
		FWK_ASSERT_RETURN_VALUE_IF_FAILED(!m_binaryConverter.SaveStaticModelAsset(a_filePath, *l_staticModelRecord), "TextureAssetの保存に失敗しました", {});
	}

	return l_staticModelLoadResult;
}