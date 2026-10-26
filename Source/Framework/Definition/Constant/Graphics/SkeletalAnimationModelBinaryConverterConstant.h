#pragma once

namespace FWK::Constant
{
    inline constexpr std::uint64_t k_emptyBonePaletteCount    = 0ULL;
    inline constexpr std::uint64_t k_emptyBoneCount           = 0ULL;
    inline constexpr std::uint64_t k_emptyMotionSequenceCount = 0ULL;

    // SkeletalModelのバイナリーファイル用拡張子
    // StaticModelと同じFBXから作っても別ファイルになるように、モデルの種類ごとに拡張子を分ける
    inline const std::filesystem::path k_skeletalModelAssetExtension = ".skeletalModel";
}