#pragma once

namespace FWK::Graphics
{
    // StandardUnLit(ライティングなし)シェーダー用のMaterial
    // ライティングを行わないため、ベースカラーのテクスチャと係数だけを持つ
    class StandardUnLitMaterial final : public MaterialBase
    {
    public:

         StandardUnLitMaterial()          = default;
        ~StandardUnLitMaterial() override = default;

        bool Deserialize(const nlohmann::json& a_rootJson) override;

        void BuildRuntimeResources() override;

        bool CreateGPUDataBuffer(const Device&                                                   a_device,
                                 const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                       std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                       TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool) override;

        nlohmann::json Serialize() const override;

        std::uint64_t CalculateBinaryPayloadSize(const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter)                                          const override;
        void          WriteBinaryPayload        (const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryWriteOffset) const override;
        bool          ReadBinaryPayload         (const Converter::MaterialBaseBinaryConverter& a_materialBaseBinaryConverter, std::uint64_t& a_memoryReadOffset)        override;

        bool EditInspectorParameter() override;

        void SetBaseColorFactor         (const TypeAlias::Math::Color& a_set) { m_baseColorFactor          = a_set; }
        void SetBaseColorTextureFilePath(const std::wstring&           a_set) { m_baseColorTextureFilePath = a_set; }

        const auto& GetREFBaseColorFactor         () const { return m_baseColorFactor; }
        const auto& GetREFBaseColorTextureFilePath() const { return m_baseColorTextureFilePath; }

    private:

        Texture m_baseColorTexture = {};

        StandardUnLitMaterialEditInspector m_editInspector = {};

        Converter::StandardUnLitMaterialBinaryConverter m_binaryConverter = {};
        Converter::StandardUnLitMaterialJsonConverter   m_jsonConverter   = {};

        std::wstring m_baseColorTextureFilePath = {};

        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

        FWK_DEFINE_TYPE_INFO(StandardUnLitMaterial, MaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MaterialSharedFactory, FWK::Graphics::StandardUnLitMaterial)