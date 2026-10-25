#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelMorphSetting;
}

namespace FWK::Converter
{
    // SkeletalAnimationModelMorphSettingを.morphファイルへ保存、読み込みするクラス
    // .morphファイルはFBXと同じ場所・同じ名前で、拡張子だけを.morphにしたファイル
    class SkeletalAnimationModelMorphSettingBinaryConverter final : public BinaryConverterBase
    {
    private:

        // ※ ファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
        struct MorphSettingBinaryHeader final
        {
            std::uint64_t m_fileSize           = k_emptyAssetFileSize;
            std::uint64_t m_targetSettingCount = k_emptyTargetSettingCount;
            std::uint32_t m_assetTypeID        = k_morphSettingAssetTypeID;
            std::uint16_t m_version            = k_morphSettingAssetVersion;
            std::uint16_t m_padding            = k_initialPadding;
        };

        // ターゲット1つ分の、名前以外の設定
        // ※ ファイルにこの並びのまま保存するため、メンバを変えたらバージョンを上げる
        struct TargetSettingBinaryData final
        {
            std::uint64_t m_targetNameSize = k_emptyTargetNameSize;

            Enum::ModelMorphLODProtection m_lodProtection = Enum::ModelMorphLODProtection::Normal;

            float m_defaultWeight = Constant::k_defaultModelMorphTargetWeight;
            float m_minWeight     = Constant::k_defaultModelMorphTargetMINWeight;
            float m_maxWeight     = Constant::k_defaultModelMorphTargetMAXWeight;

            std::uint32_t m_isShadowEnabled = k_trueValue;
        };

    public:

         SkeletalAnimationModelMorphSettingBinaryConverter()          = default;
        ~SkeletalAnimationModelMorphSettingBinaryConverter() override = default;

        // a_modelFilePath : モデル(FBX)のパス(.morphファイルのパスはここから作る)
        // .morphファイルが無い場合はfalseを返す
        bool Load(Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting, const std::filesystem::path& a_modelFilePath);

        bool Save(const Graphics::SkeletalAnimationModelMorphSetting& a_morphSetting, const std::filesystem::path& a_modelFilePath);

        // .morphファイルが存在するか
        bool Exists(const std::filesystem::path& a_modelFilePath) const;

    private:

        std::uint64_t CalculateFileSize(const std::vector<Struct::ModelMorphTargetSetting>& a_targetSettingList) const;

        static constexpr std::uint32_t k_morphSettingAssetTypeID = 0x46524F4DU;

        static constexpr std::uint16_t k_morphSettingAssetVersion = 1U;

        static constexpr std::uint16_t k_initialPadding = 0U;

        static constexpr std::uint64_t k_emptyTargetSettingCount = 0ULL;
        static constexpr std::uint64_t k_emptyTargetNameSize     = 0ULL;

        static constexpr std::uint32_t k_trueValue  = 1U;
        static constexpr std::uint32_t k_falseValue = 0U;
    };
}