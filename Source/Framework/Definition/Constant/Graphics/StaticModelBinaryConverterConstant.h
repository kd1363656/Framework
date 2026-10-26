#pragma once

namespace FWK::Constant
{
    // StaticModelのバイナリーファイル用拡張子
    // SkeletalModelと同じFBXから作っても別ファイルになるように、モデルの種類ごとに拡張子を分ける
    inline const std::filesystem::path k_staticModelAssetExtension = ".staticModel";
}
