#pragma once

namespace FWK::Enum
{
    // EditorWindowClipboardに入っている要素を、貼り付け時にどう扱うか
    // AssetBrowser(ファイル)とWorldOutliner(GameObject)で共通
    enum class EditorWindowClipboardOperationType
    {
        Invalid,
        Copy,
        Cut
    };
}