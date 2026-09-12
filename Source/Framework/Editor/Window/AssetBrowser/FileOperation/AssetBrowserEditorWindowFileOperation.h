#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFileOperation final
    {
    public:

         AssetBrowserEditorWindowFileOperation() = default;
        ~AssetBrowserEditorWindowFileOperation() = default;

        void Rename(const std::filesystem::path& a_targetFilePath, const std::string& a_newName, AssetFilePathRegistry& a_assetFilePathRegistry);

        void Delete(const std::vector<std::filesystem::path>& a_filePathList);

        void Copy(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard);
        void Cut (const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard);

        void Paste(const std::filesystem::path& a_destinationFolderPath, AssetBrowserEditorWindowClipboard& a_clipboard);

        void Duplicate(const std::vector<std::filesystem::path>& a_filePathList);
    };
}
