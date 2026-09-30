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

        void EditInspector(AssetFilePath& a_assetFilePath) const;

    private:

        static constexpr std::string_view k_assetFilePathDragDropAreaID    = "##AssetFilePathDragDrop";
        static constexpr std::string_view k_assetFilePathDropAreaEmptyText = "コンテンツブラウザーからアセットファイルをドロップ";

        static constexpr float k_assetFilePathDropAreaHeight   = 48.0F;
        static constexpr float k_assetFilePathDropAreaMINWidth = 1.0F;
    };
}