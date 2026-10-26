#pragma once

namespace FWK::Struct
{
    // Utility::FileDialogで表示するダイアログの設定
    // 別スレッドへコピーして渡すため、文字列はstd::wstring_viewではなくstd::wstringで持つ
    struct FileDialogDesc final
    {
        // nullptrなら親ウィンドウなしで表示する
        // (親ウィンドウを指定すると、ダイアログを閉じるまで親ウィンドウは操作できなくなる)
        HWND m_ownerHWND = nullptr;

        // 空、又は存在しないフォルダならWindowsが決めたフォルダで開く
        std::filesystem::path m_initialDirectoryPath = {};

        std::wstring m_title            = {};
        std::wstring m_filterName       = {};
        std::wstring m_filterPattern    = {};
        std::wstring m_defaultExtension = {};
    };
}