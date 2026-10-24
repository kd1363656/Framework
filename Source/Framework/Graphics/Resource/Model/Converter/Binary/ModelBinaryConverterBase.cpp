#include "ModelBinaryConverterBase.h"

bool FWK::Converter::ModelBinaryConverterBase::CanLoadAsset(const std::filesystem::path& a_filePath, const std::filesystem::path& a_assetExtension) const
{
    // 元となるFBXが存在しない場合は、バイナリーファイルの正当性を判断できないので読み込まない
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerFBXExtension)) { return false; }

    // a_assetExtensionはStaticModelなら.staticModel、SkeletalModelなら.skeletalModel
    // 同じFBXからでもモデルの種類ごとに別ファイルとして保存されるので、互いに上書きしない
    const auto& l_modelAssetFilePath = CreateAssetFilePath(a_filePath, a_assetExtension);

    // バイナリーファイルが存在しないなら、FBXから読み込んで生成する
    if (!Utility::CanLoadFilePath(l_modelAssetFilePath, a_assetExtension)) { return false; }

    // FBXがバイナリーファイルより新しいなら、古いバイナリーファイルは使わない
    if (IsUpdatedSourceFile(a_filePath, l_modelAssetFilePath)) { return false; }

    return true;
}