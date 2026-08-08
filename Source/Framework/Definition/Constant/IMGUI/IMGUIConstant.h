#pragma once

namespace FWK::Constant
{
	inline constexpr std::string_view k_selecteUnknownString = "Unknown";
	inline constexpr std::string_view k_isTrueString         = "True";
	inline constexpr std::string_view k_isFalseString        = "False";

	inline constexpr std::string_view k_gameObjectString = "GameObject";

	inline constexpr std::string_view k_assetFilePathDragAndDropPayloadLabel = "AssetFilePath";

	inline constexpr float k_imguiDefaultDragValue       = 0.10F;
	inline constexpr float k_imguiDefaultDelayDrawSecond = 1.0F;

	inline constexpr std::size_t k_filePathNullTerminatorElementCount = 1ULL;
}