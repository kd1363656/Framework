#pragma once

namespace FWK::Graphics
{
    class StandardUnLitMaterial;
}

namespace FWK::Converter
{
    // StandardUnLitMaterial固有のデータ(Payload)を.materialファイルへ読み書きするクラス
    // ファイルの開閉とヘッダー、型名の読み書きはMaterialBaseBinaryConverterが行い、
    // このクラスはMaterialBaseBinaryConverterの読み書き用の関数でPayloadだけを読み書きする
    // ※ ファイルを持たないため、BinaryConverterBaseは継承しない
    class StandardUnLitMaterialBinaryConverter final
    {
    public:

         StandardUnLitMaterialBinaryConverter() = default;
        ~StandardUnLitMaterialBinaryConverter() = default;

        // 保存順を変えた場合は、MaterialBaseBinaryConverterのバージョンを上げる
        std::uint64_t CalculatePayloadSize(const Graphics::StandardUnLitMaterial& a_standardUnLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter)                                          const;
        void          WritePayload        (const Graphics::StandardUnLitMaterial& a_standardUnLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const;
        bool          ReadPayload         (      Graphics::StandardUnLitMaterial& a_standardUnLitMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset)  const;
    };
}