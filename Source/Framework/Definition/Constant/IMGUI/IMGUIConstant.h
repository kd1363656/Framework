#pragma once

namespace FWK::Constant
{
	inline constexpr std::string_view k_selecteUnknownString = "Unknown";
	inline constexpr std::string_view k_isTrueString         = "True";
	inline constexpr std::string_view k_isFalseString        = "False";

	inline constexpr std::string_view k_gameObjectString = "GameObject";

	inline constexpr std::string_view k_assetFilePathDragAndDropPayloadLabel = "AssetFilePath";

	inline constexpr std::string_view k_fontAwesomeFolderIcon = "\xEF\x81\xBB";
	inline constexpr std::string_view k_fontAwesomeFileIcon   = "\xEF\x85\x9B";
	inline constexpr std::string_view k_fontAwesomeImageIcon  = "\xEF\x80\xBE";
	inline constexpr std::string_view k_fontAwesomeCubeIcon   = "\xEF\x86\xB2";

	inline constexpr float k_imguiDefaultDragValue       = 0.10F;
	inline constexpr float k_imguiDefaultDelayDrawSecond = 1.0F;

	inline constexpr std::size_t k_filePathNullTerminatorElementCount = 1ULL;
}