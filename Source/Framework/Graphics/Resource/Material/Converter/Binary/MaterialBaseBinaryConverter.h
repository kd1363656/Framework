#pragma once

namespace FWK::Converter
{
    // .materialファイルの読み書きを行うクラス
    // ファイルの先頭にはヘッダーとMaterialの型名を保存し、
    // その後に続くPayload(Material固有のデータ)は各Material自身が読み書きする
    // ※ PayloadはMaterialBaseの派生クラスから読み書きするため、読み書き用の関数を公開している
    class MaterialBinaryConverter final : public BinaryConverterBase
    {
    private:

        static constexpr std::uint16_t k_initialPadding = 0U;

        struct MaterialBinaryHeader final
        {
            std::uint64_t m_fileSize     = k_emptyAssetFileSize;
            std::uint64_t m_typeNameSize = k_emptyTypeNameSize;
            std::uint32_t m_assetTypeID  = k_materialAssetTypeID;
            std::uint16_t m_version      = k_materialAssetVersion;
            std::uint16_t m_padding      = k_initialPadding;
        };

    public:

         MaterialBinaryConverter()          = default;
        ~MaterialBinaryConverter() override = default;

        // ファイルに保存されている型名だけを読み込む(どの型のMaterialを作るか決めるために使う)
        bool TryReadTypeName(const std::filesystem::path& a_filePath, std::string& a_typeName);

        // ファイルを開いてヘッダーと型名を読み込む
        // 続けてPayloadを読む場合は、a_memoryReadOffsetをそのまま使う
        bool BeginRead(const std::filesystem::path& a_filePath, std::string& a_typeName, std::uint64_t& a_memoryReadOffset);

        // Payloadのサイズ分を含めたファイルを作成し、ヘッダーと型名を書き込む
        bool BeginWrite(const std::filesystem::path& a_filePath,
                        const std::string_view&      a_typeName,
                        const std::uint64_t&         a_payloadSize,
                              std::uint64_t&         a_memoryWriteOffset);

        // 読み込み、書き込みを終える
        // ファイルサイズと最後に読み書きした位置が一致しなければ、壊れたファイルとして失敗を返す
        bool EndRead (const std::uint64_t& a_memoryReadOffset);
        bool EndWrite(const std::uint64_t& a_memoryWriteOffset);

        template <typename Type>
        bool TryReadSingle(Type& a_destinationData, std::uint64_t& a_memoryReadOffset) const
        {
            return TryReadSingleBinaryData(a_destinationData, a_memoryReadOffset);
        }

        // 文字列のサイズ(バイト数)を読み込んでから、文字列本体を読み込む
        bool TryReadWString(std::wstring& a_destinationString, std::uint64_t& a_memoryReadOffset) const;

        template <typename Type>
        void WriteSingle(const Type& a_sourceData, std::uint64_t& a_memoryWriteOffset) const
        {
            WriteBinaryData(k_singleBinaryElementCount, &a_sourceData, a_memoryWriteOffset);
        }

        // 文字列のサイズ(バイト数)を書き込んでから、文字列本体を書き込む
        void WriteWString(const std::wstring& a_wString, std::uint64_t& a_memoryWriteOffset) const;

        template <typename Type>
        std::uint64_t CalculateSingleSize() const
        {
            return CalculateBinaryDataSize<Type>(k_singleBinaryElementCount);
        }

        // 文字列のサイズ情報を含めた、文字列の保存サイズを計算する
        std::uint64_t CalculateWStringSize(const std::wstring& a_wString) const;

    private:

        static constexpr std::uint32_t k_materialAssetTypeID = 0x4C54414DU;

        // 保存形式を変えた場合はバージョンを上げる
        static constexpr std::uint16_t k_materialAssetVersion = 1U;

        static constexpr std::uint64_t k_emptyTypeNameSize          = 0ULL;
        static constexpr std::uint64_t k_emptyWStringBinaryFileSize = 0ULL;
    };
}