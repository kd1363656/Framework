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

    // PopupDrawerが描画するポップアップのコンテキスト種別
    // どのPaneのどの位置で右クリックされたかによって
    // 表示するメニュー項目と有効/無効が変わる
    enum class AssetBrowserPopupContextType
    {
        Invalid,

        // FolderPane
        // フォルダ上で右クリック -> 全メニュー有効
        FolderPane_OnFolder,
        // 空白で左クリック -> 新規フォルダ・名前変更は色薄(disabled)
        FolderPane_OnEmpty,

        // AssetPane
        // フォルダ上で右クリック -> 全メニュー有効
        AssetPane_OnFolder,
        // ファイル上で右クリック -> 新規フォルダ・名前変更は色薄(disabled)
        AssetPane_OnFile,
        // 空白で右クリック -> 作成(新規フォルダ/新規プレハブ/新規シーン)のみ
        AssetPane_OnEmpty,
    };
}