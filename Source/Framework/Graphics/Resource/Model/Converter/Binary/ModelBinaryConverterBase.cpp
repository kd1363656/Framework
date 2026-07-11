#include "ModelBinaryConverterBase.h"

bool FWK::Converter::ModelBinaryConverterBase::CanLoadAsset(const std::filesystem::path& a_filePath) const
{
    	// 元となるFBXが存在しない場合は、.assetの正当性を判断できないので読み込まない
	if (!Utility::CanLoadFilePath(a_filePath, Graphics::FBXModelLoaderBase::k_lowerFBXExtension)) { return false; }

	const auto& l_modelAssetFilePath = CreateAssetFilePath(a_filePath);

	// .assetが存在しないなら、FBXから読み込んで生成する
	if (!Utility::CanLoadFilePath(l_modelAssetFilePath, BinaryConverterBase::k_lowerAssetExtension)) { return false; }

	// FBXが.assetより新しいなら、古い.assetは使わない
	if (IsUpdatedSourceFile(a_filePath, l_modelAssetFilePath)) { return false; }

	return true;
}