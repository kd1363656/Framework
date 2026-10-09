#pragma once

namespace FWK
{
    class AssetFilePath;
}

namespace FWK
{
    class AssetFilePathInspector final
    {
    public:

         AssetFilePathInspector() = default;
        ~AssetFilePathInspector() = default;

        bool EditInspector(AssetFilePath& a_assetFilePath) const;

    private:

        void RegisterTextureFilePath(const std::filesystem::path& a_filePath, AssetFilePathRegistry& a_assetFilePathRegistry) const;

        bool ApplyDroppedFilePath(const std::filesystem::path& a_droppedFilePath, AssetFilePath& a_assetFilePath) const;

        std::string FetchVALButtonText(const std::filesystem::path& a_filePath) const;

        static constexpr std::string_view k_assetFilePathDragDropAreaID = "##AssetFilePathDragDrop";
        static constexpr std::string_view k_unknownFilePathText         = "不明なパス";

        static constexpr float k_assetFilePathDropAreaHeight   = 48.0F;
        static constexpr float k_assetFilePathDropAreaMINWidth = 1.0F;

        static constexpr std::size_t k_singleDroppedFileCount = 1ULL;
    };
}