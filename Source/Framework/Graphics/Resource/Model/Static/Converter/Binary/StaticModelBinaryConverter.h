#pragma once

namespace FWK::Converter
{
    class StaticModelBinaryConverter final : public ModelBinaryConverterBase
    {
    private:

        struct ModelBinaryHeader final
        {
            std::uint64_t m_fileSize    = k_emptyAssetFileSize;
            std::uint16_t m_version     = k_modelAssetVersion;
            std::uint16_t m_assetTypeID = k_modelAssetTypeID;

            std::uint64_t m_modelMeshCount = Constant::k_emptyModelMeshCount;
        };

    public:

         StaticModelBinaryConverter()          = default;
        ~StaticModelBinaryConverter() override = default;

        bool LoadAsset(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord);

        bool SaveAsset(const Graphics::StaticModelRecord& a_staticModelRecord, const std::filesystem::path& a_filePath);

    private:

        void FailLoadAsset(Graphics::StaticModelRecord::ModelData& a_modelData);

        ModelBinaryHeader CreateModelBinaryHeader(const Graphics::StaticModelRecord::ModelData& a_modelData, const std::uint64_t& a_fileSize) const;

        std::uint64_t CalculateAssetFileSize(const Graphics::StaticModelRecord::ModelData& a_modelData) const;

        // StaticModelのバイナリーファイル用拡張子
        // SkeletalModelと同じFBXから作っても別ファイルになるように、モデルの種類ごとに拡張子を分ける
        inline static const std::filesystem::path k_staticModelAssetExtension = ".staticModel";

        // 'S' = 0x53, 'T' = 0x54のため、0x5354で"ST"を表す
        static constexpr std::uint16_t k_modelAssetTypeID = 0x5354U;

        // ※ 注意 : Assetとして保存する構造体が変化したらバージョンを上げる
        // Version2 : LOD一覧とMesh全体の囲み球を追加
        // Version3 : 遠距離用LODで継ぎ目をまたいだ簡略化を許可(LODの作り方の変更)
        // Version4 : 遠距離用LODでSloppy簡略化を使い、LODの最大数を9に増加(LODの作り方の変更)
        // Version5 : 簡略化で三角形が減らない場合は Permissive → Sloppy の順に試すように変更(LODの作り方の変更)
        // Version6 : Materialを.materialファイルへ分け、.materialファイルのパスだけを保存するように変更
        // Version7 : LODの簡略化で法線とUVのずれも誤差に含め、Sloppy簡略化をやめた(LODの作り方の変更)
        static constexpr std::uint16_t k_modelAssetVersion = 7U;
    };
}