#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowAssetCreator;
}

namespace FWK::Editor
{
    class AssetBrowserEditorWindowFileOperation final
    {
    public:

         AssetBrowserEditorWindowFileOperation() = default;
        ~AssetBrowserEditorWindowFileOperation() = default;

        void Rename(const std::filesystem::path&                a_targetFilePath, 
                    const std::string&                          a_newName, 
                    const AssetBrowserEditorWindowAssetCreator& a_assetCreator,
                          AssetFilePathRegistry&                a_assetFilePathRegistry) const;

        void Delete(const std::vector<std::filesystem::path>& a_filePathList) const;

        void Copy(const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard) const;
        void Cut (const std::vector<std::filesystem::path>& a_filePathList, AssetBrowserEditorWindowClipboard& a_clipboard) const;

        void Paste(const std::vector<std::filesystem::path>&   a_destinationFolderPathList,
                   const AssetBrowserEditorWindowAssetCreator& a_assetCreator,
                         AssetBrowserEditorWindowClipboard&    a_clipboard,            
                         AssetFilePathRegistry&                a_assetFilePathRegistry) const;

        void Duplicate(const std::vector<std::filesystem::path>& a_filePathList, const AssetBrowserEditorWindowAssetCreator& a_assetCreator, AssetFilePathRegistry& a_assetFilePathRegistry) const;

        void Move(const std::vector<std::filesystem::path>& a_sourceFilePathList, const std::filesystem::path& a_destinationFolderPath) const;

    private:

        static void CopyRecursiveSkippingDestination(const std::filesystem::path& a_source, const std::filesystem::path& a_destination, const std::filesystem::path& a_topDestination);

        static void RegisterCopiedAssetList(const std::filesystem::path&                a_sourceRootPath,
                                            const std::filesystem::path&                a_copiedRootPath,
                                            const AssetBrowserEditorWindowAssetCreator& a_assetCreator,
                                                  AssetFilePathRegistry&                a_assetFilePathRegistry);
    };
}
