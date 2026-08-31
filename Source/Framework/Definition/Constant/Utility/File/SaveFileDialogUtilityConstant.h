#pragma once

namespace FWK::Constant
{
	inline const std::filesystem::path k_contentRootFolderPath = "Content";

	inline constexpr std::wstring_view k_saveJsonFileDialogTitle        = L"シーンファイルを保存";
	inline constexpr std::wstring_view k_jsonFileDialogFilterName       = L"JSON File(*.json)";
	inline constexpr std::wstring_view k_jsonFileDialogFilterPattern    = L"*.json";
	inline constexpr std::wstring_view k_jsonFileDialogDefaultExtension = L"json";

	inline constexpr UINT k_defaultSaveFileDialogCount = 1U;
}