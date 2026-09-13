#pragma once

namespace FWK::Struct
{
    struct AssetBrowserEditorWindowRenameState final
    {
        // 名前変更対象のファイルパス
        // 空の場合は名前変更モードではない
        std::filesystem::path m_targetFilePath = {};

        // InputText用のバッファ
        // ImGui::InputTextが直接編集するためchar配列
        std::array<char, Constant::k_assetBrowserRenameInputTextBufferSize> m_inputBuffer = {};

        // 名前変更モード中かどうか
        bool m_isActive = false;
    };
}