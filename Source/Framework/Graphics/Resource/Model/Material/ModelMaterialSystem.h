#pragma once

namespace FWK::Graphics
{
    class ModelMaterialSystem final
    {
    public:

         ModelMaterialSystem() = default;
        ~ModelMaterialSystem() = default;

        void Deserialize(const nlohmann::json& a_rootJson);
        bool Create     ();

        nlohmann::json Serialize() const;

        bool CreateErrorMaterial();

        std::weak_ptr<ModelMaterialRecord> LoadModelMaterial(const std::filesystem::path& a_filePath);

        bool AddModelMaterialReferenceCount     (const std::weak_ptr<ModelMaterialRecord>& a_materialRecord);
        bool SubtractModelMaterialReferenceCount(const std::weak_ptr<ModelMaterialRecord>& a_materialRecord);

        const auto& GetREFErrorMaterial() const { return m_errorMaterial; }

        const auto& GetREFMaterialStorage() const { return m_materialStorage; }

        auto& GetMutableREFMaterialStorage() { return m_materialStorage; }

    private:

        std::shared_ptr<ModelMaterialBase> CreateModelMaterial(const std::filesystem::path& a_filePath);

        std::shared_ptr<ModelMaterialBase> m_errorMaterial = nullptr;

        AssetStorage<ModelMaterialRecord> m_materialStorage = {};

        Converter::ModelMaterialSystemJsonConverter m_jsonConverter   = {};
        Converter::ModelMaterialBinaryConverter     m_binaryConverter = {};
    };
}