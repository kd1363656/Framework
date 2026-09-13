#pragma once

namespace FWK::Enum
{
    enum class AssetBrowserPopupContextType
    {
        Invalid,

        FolderPane_OnFolder,
        FolderPane_OnEmpty,

        AssetPane_OnFolder,
        AssetPane_OnFile,
        AssetPane_OnEmpty,
    };
}