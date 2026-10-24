#pragma once

namespace FWK::Graphics
{
    // StandardLit(PBRライティングあり)シェーダー用のMaterial
    // ベースカラー、法線、ラフネス、メタリックのテクスチャと係数を持つ
    class StandardLitMaterial final : public MaterialBase
    {
    public:

         StandardLitMaterial()          = default;
        ~StandardLitMaterial() override = default;

        void BuildRuntimeResources() override;

        bool CreateGPUDataBuffer(const Device&                                                   a_device,
                                 const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                       std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                       TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool) override;

        void SetBaseColorFactor(const TypeAlias::Math::Color& a_set) { m_baseColorFactor = a_set; }

        void SetBaseColorTextureFilePath(const std::wstring& a_set) { m_baseColorTextureFilePath = a_set; }
        void SetNormalTextureFilePath   (const std::wstring& a_set) { m_normalTextureFilePath    = a_set; }
        void SetRoughnessTextureFilePath(const std::wstring& a_set) { m_roughnessTextureFilePath = a_set; }
        void SetMetallicTextureFilePath (const std::wstring& a_set) { m_metallicTextureFilePath  = a_set; }

        void SetRoughnessFactor(const float a_set) { m_roughnessFactor = a_set; }
        void SetMetallicFactor (const float a_set) { m_metallicFactor  = a_set; }

        const auto& GetREFBaseColorFactor() const { return m_baseColorFactor; }

        const auto& GetREFBaseColorTextureFilePath() const { return m_baseColorTextureFilePath; }
        const auto& GetREFNormalTextureFilePath   () const { return m_normalTextureFilePath; }
        const auto& GetREFRoughnessTextureFilePath() const { return m_roughnessTextureFilePath; }
        const auto& GetREFMetallicTextureFilePath () const { return m_metallicTextureFilePath; }

        auto GetVALRoughnessFactor() const { return m_roughnessFactor; }
        auto GetVALMetallicFactor () const { return m_metallicFactor; }

    private:

        std::uint64_t CalculateBinaryPayloadSize(const Converter::MaterialBinaryConverter& a_materialBinaryConverter)                                          const override;
        void          WriteBinaryPayload        (const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryWriteOffset) const override;
        bool          ReadBinaryPayload         (const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryReadOffset)        override;

        bool EditInspectorParameter() override;

        Texture m_baseColorTexture = {};
        Texture m_normalTexture    = {};
        Texture m_roughnessTexture = {};
        Texture m_metallicTexture  = {};

        StandardLitMaterialInspector m_inspector = {};

        // .materialファイルの場所を基準にした相対パス
        std::wstring m_baseColorTextureFilePath = {};
        std::wstring m_normalTextureFilePath    = {};
        std::wstring m_roughnessTextureFilePath = {};
        std::wstring m_metallicTextureFilePath  = {};

        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

        float m_roughnessFactor = Constant::k_defaultStandardMaterialRoughnessFactor;
        float m_metallicFactor  = Constant::k_defaultStandardMaterialMetallicFactor;

        FWK_DEFINE_TYPE_INFO(StandardLitMaterial, MaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MaterialSharedFactory, FWK::Graphics::StandardLitMaterial)