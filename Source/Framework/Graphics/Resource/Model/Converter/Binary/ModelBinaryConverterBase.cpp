#include "ModelBinaryConverterBase.h"

bool FWK::Converter::ModelBinaryConverterBase::CanLoad(const std::filesystem::path& a_filePath, const std::filesystem::path& a_modelAssetFilePath) const
{
        // 元となるFBXが存在しない場合は、.assetの正当性を判断できないので読み込まない
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension)) { return false; }

    const auto& l_modelAssetFilePath = CreateAssetFilePath(a_filePath);

    // .assetが存在しないなら、FBXから読み込んで生成する
    if (!Utility::CanLoadFilePath(l_modelAssetFilePath, Constant::k_lowerAssetExtension)) { return false; }

    // FBXが.assetより新しいなら、古い.assetは使わない
    if (IsUpdatedSourceFile(a_filePath, l_modelAssetFilePath)) { return false; }

    return true;
}