#pragma once

namespace FWK::Graphics
{
    class ModelMaterialRecord final : public AssetRecordBase
    {
    public:

         ModelMaterialRecord()          = default;
        ~ModelMaterialRecord() override = default;

        ModelMaterialRecord(const ModelMaterialRecord&)           = delete;
        ModelMaterialRecord(      ModelMaterialRecord&&) noexcept = default;

        ModelMaterialRecord& operator=(const ModelMaterialRecord&)           = delete;
        ModelMaterialRecord& operator=(      ModelMaterialRecord&&) noexcept = default;

        void SetMaterial(const std::shared_ptr<ModelMaterialBase>& a_set) { m_material = a_set; }

        const auto& GetREFMaterial() const { return m_material; }

    private:

        std::shared_ptr<ModelMaterialBase> m_material = nullptr;
    };
}