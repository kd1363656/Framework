#pragma once

namespace FWK::Constant
{
    inline const std::filesystem::path k_assetRootFolderPath = "Asset";

    inline constexpr std::wstring_view k_jsonFileDialogFilterName       = L"JSON File(*.json)";
    inline constexpr std::wstring_view k_jsonFileDialogFilterPattern    = L"*.json";
    inline constexpr std::wstring_view k_jsonFileDialogDefaultExtension = L"json";

    inline constexpr std::size_t k_fileDialogFilterCount = 1ULL;
}