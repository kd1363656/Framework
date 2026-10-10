#pragma once

namespace FWK::Graphics
{
    class ModelStandardLitMaterial final : public ModelMaterialBase
    {
    public:

         ModelStandardLitMaterial();
        ~ModelStandardLitMaterial() override;

        bool TryReadBinaryData(const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryReadOffset)        override;
        void WriteBinaryData  (const Converter::ModelMaterialBinaryConverter& a_binaryConverter, std::uint64_t& a_memoryWriteOffset) const override;

        std::uint64_t CalculateBinaryDataSize(const Converter::ModelMaterialBinaryConverter& a_binaryConverter) const override;

        void LoadRuntimeTextures() override;

        void SetNormalTexture   (const AssetFilePath& a_set) { m_normalTexture    = a_set; }
        void SetMetallicTexture (const AssetFilePath& a_set) { m_metallicTexture  = a_set; }
        void SetRoughnessTexture(const AssetFilePath& a_set) { m_roughnessTexture = a_set; }

        void SetMetallic (const float a_set) { m_metallic  = a_set; }
        void SetRoughness(const float a_set) { m_roughness = a_set; }

        const Struct::ModelRenderTableINFO& FetchREFTableINFO() const override;

    protected:

        void WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const override;

    private:

        static constexpr std::uint64_t k_litTextureCount = 3ULL;
        static constexpr std::uint64_t k_litValueCount   = 2ULL;

        AssetFilePath m_normalTexture;
        AssetFilePath m_metallicTexture;
        AssetFilePath m_roughnessTexture;

        Texture m_normalRuntimeTexture;
        Texture m_metallicRuntimeTexture;
        Texture m_roughnessRuntimeTexture;

        float m_metallic;
        float m_roughness;

        FWK_DEFINE_TYPE_INFO(ModelStandardLitMaterial, ModelMaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ModelMaterialSharedFactory, FWK::Graphics::ModelStandardLitMaterial)