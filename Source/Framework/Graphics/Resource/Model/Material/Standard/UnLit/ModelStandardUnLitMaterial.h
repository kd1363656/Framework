#pragma once

namespace FWK::Graphics
{
    class ModelStandardUnLitMaterial final : public ModelMaterialBase
    {
    public:

         ModelStandardUnLitMaterial()          = default;
        ~ModelStandardUnLitMaterial() override = default;

        const Struct::ModelRenderTableINFO& FetchREFTableINFO() const override;

    protected:

        void WriteGPUData(GPUElementTable& a_table, const std::uint32_t a_tableElementIndex) const override;

    private:

        FWK_DEFINE_TYPE_INFO(ModelStandardUnLitMaterial, ModelMaterialBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ModelMaterialSharedFactory, FWK::Graphics::ModelStandardUnLitMaterial)