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

        void BuildRuntimeResources() override;

        bool CreateGPUDataBuffer(const Device&                                                   a_device,
                                 const GPUMemoryAllocator&                                       a_gpuMemoryAllocator,
                                       std::vector<StaticStructuredBuffer::BufferUploadCommand>& a_bufferUploadCommandList,
                                       TypeAlias::CBVSRVUAVDescriptorPool&                       a_cbvSRVUAVDescriptorPool) override;

        void SetBaseColorFactor         (const TypeAlias::Math::Color& a_set) { m_baseColorFactor          = a_set; }
        void SetBaseColorTextureFilePath(const std::wstring&           a_set) { m_baseColorTextureFilePath = a_set; }

        const auto& GetREFBaseColorFactor         () const { return m_baseColorFactor; }
        const auto& GetREFBaseColorTextureFilePath() const { return m_baseColorTextureFilePath; }

    private:

        std::uint64_t CalculateBinaryPayloadSize(const Converter::MaterialBinaryConverter& a_materialBinaryConverter)                                          const override;
        void          WriteBinaryPayload        (const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryWriteOffset) const override;
        bool          ReadBinaryPayload         (const Converter::MaterialBinaryConverter& a_materialBinaryConverter, std::uint64_t& a_memoryReadOffset)        override;

        bool EditInspectorParameter() override;

        Texture m_baseColorTexture = {};

        StandardUnLitMaterialInspector m_inspector = {};

        std::wstring m_baseColorTextureFilePath = {};

        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultStandardMaterialBaseColorFactor;

        FWK_DEFINE_TYPE_INFO(StandardUnLitMaterial, MaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MaterialSharedFactory, FWK::Graphics::StandardUnLitMaterial)