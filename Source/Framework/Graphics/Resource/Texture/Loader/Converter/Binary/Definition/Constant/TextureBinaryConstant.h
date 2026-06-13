#pragma once

namespace FWK::Constant
{
	inline const std::filesystem::path k_lowerPNGExtension	 = ".png";
	inline const std::filesystem::path k_lowerAssetExtension = ".asset";

	inline constexpr std::uint64_t k_emptyAssetFileSize = 0ULL;

	inline constexpr std::uint64_t k_defaultTexture2DArraySize = 1ULL;
	inline constexpr std::uint64_t k_defaultTexture2DMipLevels = 1ULL;

	inline constexpr std::uint32_t k_emptyTextureWidth  = 0U;
	inline constexpr std::uint32_t k_emptyTextureHeight = 0U;

	inline constexpr std::uint16_t k_emptyAssetVersion = 0U;
	inline constexpr std::uint16_t k_emptyAssetTypeID  = 0U;
}