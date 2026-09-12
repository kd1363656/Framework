#pragma once

namespace FWK::Enum
{
    enum class AssetBrowserFileClipboardOperationType
    {
        Invalid,
        Copy,
        Cut
    };

    enum class AssetBrowserInputCommandType
    {
        Invalid,
        Copy,
        Cut,
        Paste,
        Delete,
        Rename,
    };
}