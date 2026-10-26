#pragma once

namespace FWK::Graphics
{
    // RitchToon(トゥーン調)シェーダー用のMaterial
    // lilToonを参考に、くっきりした影、光沢、マットキャップ、発光、輪郭線に必要な値とテクスチャだけを持つ
    //
    // ・マスクテクスチャが設定されていない場合は白(マスク無し = 全体に効く)、
    //   マットキャップが設定されていない場合は黒(何も足さない)のデフォルトテクスチャになる
    // ・光沢の色と発光色の初期値は黒(光沢も発光も無し)。使うMaterialだけ色を設定する
    // ・色は全てRGBA(Color)で持つが、シェーダーへ送るのはベースカラー以外RGBだけ
    class RitchToonMaterial final : public MaterialBase
    {
    public:

         RitchToonMaterial()          = default;
        ~RitchToonMaterial() override = default;

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

        void SetBaseColorFactor(const TypeAlias::Math::Color& a_set) { m_baseColorFactor = a_set; }
        void SetShadowColor    (const TypeAlias::Math::Color& a_set) { m_shadowColor     = a_set; }
        void SetMatcapColor    (const TypeAlias::Math::Color& a_set) { m_matcapColor     = a_set; }
        void SetSpecularColor  (const TypeAlias::Math::Color& a_set) { m_specularColor   = a_set; }
        void SetEmissionColor  (const TypeAlias::Math::Color& a_set) { m_emissionColor   = a_set; }
        void SetOutlineColor   (const TypeAlias::Math::Color& a_set) { m_outlineColor    = a_set; }

        void SetBaseColorTextureFilePath   (const std::wstring& a_set) { m_baseColorTextureFilePath    = a_set; }
        void SetNormalTextureFilePath      (const std::wstring& a_set) { m_normalTextureFilePath       = a_set; }
        void SetShadowMaskTextureFilePath  (const std::wstring& a_set) { m_shadowMaskTextureFilePath   = a_set; }
        void SetMatcapTextureFilePath      (const std::wstring& a_set) { m_matcapTextureFilePath       = a_set; }
        void SetMatcapMaskTextureFilePath  (const std::wstring& a_set) { m_matcapMaskTextureFilePath   = a_set; }
        void SetMetallicMaskTextureFilePath(const std::wstring& a_set) { m_metallicMaskTextureFilePath = a_set; }
        void SetEmissionMaskTextureFilePath(const std::wstring& a_set) { m_emissionMaskTextureFilePath = a_set; }
        void SetOutlineMaskTextureFilePath (const std::wstring& a_set) { m_outlineMaskTextureFilePath  = a_set; }

        void SetShadowBorder  (const float a_set) { m_shadowBorder   = a_set; }
        void SetShadowBlur    (const float a_set) { m_shadowBlur     = a_set; }
        void SetNormalStrength(const float a_set) { m_normalStrength = a_set; }
        void SetSmoothness    (const float a_set) { m_smoothness     = a_set; }
        void SetOutlineWidth  (const float a_set) { m_outlineWidth   = a_set; }

        void SetEmissionBaseColorBlend(const float a_set) { m_emissionBaseColorBlend = a_set; }
        void SetEmissionReplaceRate   (const float a_set) { m_emissionReplaceRate    = a_set; }

        const auto& GetREFBaseColorFactor() const { return m_baseColorFactor; }
        const auto& GetREFShadowColor    () const { return m_shadowColor; }
        const auto& GetREFMatcapColor    () const { return m_matcapColor; }
        const auto& GetREFSpecularColor  () const { return m_specularColor; }
        const auto& GetREFEmissionColor  () const { return m_emissionColor; }
        const auto& GetREFOutlineColor   () const { return m_outlineColor; }

        const auto& GetREFBaseColorTextureFilePath   () const { return m_baseColorTextureFilePath; }
        const auto& GetREFNormalTextureFilePath      () const { return m_normalTextureFilePath; }
        const auto& GetREFShadowMaskTextureFilePath  () const { return m_shadowMaskTextureFilePath; }
        const auto& GetREFMatcapTextureFilePath      () const { return m_matcapTextureFilePath; }
        const auto& GetREFMatcapMaskTextureFilePath  () const { return m_matcapMaskTextureFilePath; }
        const auto& GetREFMetallicMaskTextureFilePath() const { return m_metallicMaskTextureFilePath; }
        const auto& GetREFEmissionMaskTextureFilePath() const { return m_emissionMaskTextureFilePath; }
        const auto& GetREFOutlineMaskTextureFilePath () const { return m_outlineMaskTextureFilePath; }

        auto GetVALShadowBorder  () const { return m_shadowBorder; }
        auto GetVALShadowBlur    () const { return m_shadowBlur; }
        auto GetVALNormalStrength() const { return m_normalStrength; }
        auto GetVALSmoothness    () const { return m_smoothness; }
        auto GetVALOutlineWidth  () const { return m_outlineWidth; }

        auto GetVALEmissionBaseColorBlend() const { return m_emissionBaseColorBlend; }
        auto GetVALEmissionReplaceRate   () const { return m_emissionReplaceRate; }

    private:

        Texture m_baseColorTexture    = {};
        Texture m_normalTexture       = {};
        Texture m_shadowMaskTexture   = {};
        Texture m_matcapTexture       = {};
        Texture m_matcapMaskTexture   = {};
        Texture m_metallicMaskTexture = {};
        Texture m_emissionMaskTexture = {};
        Texture m_outlineMaskTexture  = {};

        RitchToonMaterialEditInspector m_editInspector = {};

        Converter::RitchToonMaterialBinaryConverter m_binaryConverter = {};
        Converter::RitchToonMaterialJsonConverter   m_jsonConverter   = {};

        std::wstring m_baseColorTextureFilePath    = {};
        std::wstring m_normalTextureFilePath       = {};
        std::wstring m_shadowMaskTextureFilePath   = {};
        std::wstring m_matcapTextureFilePath       = {};
        std::wstring m_matcapMaskTextureFilePath   = {};
        std::wstring m_metallicMaskTextureFilePath = {};
        std::wstring m_emissionMaskTextureFilePath = {};
        std::wstring m_outlineMaskTextureFilePath  = {};

        TypeAlias::Math::Color m_baseColorFactor = Constant::k_defaultRitchToonMaterialBaseColorFactor;
        TypeAlias::Math::Color m_shadowColor     = Constant::k_defaultRitchToonMaterialShadowColor;
        TypeAlias::Math::Color m_matcapColor     = Constant::k_defaultRitchToonMaterialMatcapColor;
        TypeAlias::Math::Color m_specularColor   = Constant::k_defaultRitchToonMaterialSpecularColor;
        TypeAlias::Math::Color m_emissionColor   = Constant::k_defaultRitchToonMaterialEmissionColor;
        TypeAlias::Math::Color m_outlineColor    = Constant::k_defaultRitchToonMaterialOutlineColor;

        float m_shadowBorder   = Constant::k_defaultRitchToonMaterialShadowBorder;
        float m_shadowBlur     = Constant::k_defaultRitchToonMaterialShadowBlur;
        float m_normalStrength = Constant::k_defaultRitchToonMaterialNormalStrength;
        float m_smoothness     = Constant::k_defaultRitchToonMaterialSmoothness;
        float m_outlineWidth   = Constant::k_defaultRitchToonMaterialOutlineWidth;

        float m_emissionBaseColorBlend = Constant::k_defaultRitchToonMaterialEmissionBaseColorBlend;
        float m_emissionReplaceRate    = Constant::k_defaultRitchToonMaterialEmissionReplaceRate;

        FWK_DEFINE_TYPE_INFO(RitchToonMaterial, MaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::MaterialSharedFactory, FWK::Graphics::RitchToonMaterial)