#pragma once

namespace FWK::Graphics
{
    class ModelMaterial final
    {
    public:

         ModelMaterial();
         ModelMaterial(const ModelMaterial&  a_other);
         ModelMaterial(      ModelMaterial&& a_other) noexcept;
        ~ModelMaterial();

        ModelMaterial& operator=(const ModelMaterial&  a_other);
        ModelMaterial& operator=(      ModelMaterial&& a_other) noexcept;

        bool Load(const std::filesystem::path& a_filePath);

        bool IsValid() const;

        std::shared_ptr<ModelMaterialBase> FetchVALMaterial() const;

    private:

        void AddReferenceCount() const;

        void SubtractReferenceCount();

        std::weak_ptr<ModelMaterialRecord> m_materialRecord;

        TypeAlias::StorageID m_storageID;
    };
}