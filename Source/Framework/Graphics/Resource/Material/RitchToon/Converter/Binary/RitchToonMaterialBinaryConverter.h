#pragma once

namespace FWK::Graphics
{
    class RitchToonMaterial;
}

namespace FWK::Converter
{
    // RitchToonMaterial固有のデータ(Payload)を.materialファイルへ読み書きするクラス
    // ファイルの開閉とヘッダー、型名の読み書きはMaterialBaseBinaryConverterが行い、
    // このクラスはMaterialBaseBinaryConverterの読み書き用の関数でPayloadだけを読み書きする
    // ※ ファイルを持たないため、BinaryConverterBaseは継承しない
    class RitchToonMaterialBinaryConverter final
    {
    public:

         RitchToonMaterialBinaryConverter() = default;
        ~RitchToonMaterialBinaryConverter() = default;

        // 保存順を変えた場合は、MaterialBaseBinaryConverterのバージョンを上げる
        std::uint64_t CalculatePayloadSize(const Graphics::RitchToonMaterial& a_ritchToonMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter)                                          const;
        void          WritePayload        (const Graphics::RitchToonMaterial& a_ritchToonMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const;
        bool          ReadPayload         (      Graphics::RitchToonMaterial& a_ritchToonMaterial, const MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset)  const;

    private:

        // WritePayload / ReadPayloadで読み書きする色(Color)と値(float)の数と必ず同じにする
        static constexpr std::uint64_t k_colorCount  = 6ULL;
        static constexpr std::uint64_t k_factorCount = 7ULL;
    };
}