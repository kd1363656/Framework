#pragma once

namespace FWK::Constant
{
    inline const std::filesystem::path k_lowerPNGExtension = ".png";

    // Textureのバイナリーファイル用拡張子
    // 同じPNGでも読み込み方法によってピクセルの扱いが変わるため、読み込み方法ごとに別ファイルとして保存する
    inline const std::filesystem::path k_autoTextureAssetExtension   = ".AutoTexture";
    inline const std::filesystem::path k_srgbTextureAssetExtension   = ".SRGBTexture";
    inline const std::filesystem::path k_linearTextureAssetExtension = ".LinearTexture";

    inline constexpr std::uint32_t k_invalidTextureWidth  = 0U;
    inline constexpr std::uint32_t k_invalidTextureHeight = 0U;

    inline constexpr std::uint64_t k_defaultTexture2DArraySize = 1ULL;
    inline constexpr std::uint64_t k_defaultTexture2DMIPLevels = 1ULL;
}