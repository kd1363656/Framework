#pragma once

namespace FWK::Graphics
{
    class ModelStandardUnLitMaterialGPUData final
    {
    public:

         ModelStandardUnLitMaterialGPUData() = default;
        ~ModelStandardUnLitMaterialGPUData() = default;

        void SetBaseColor(const TypeAlias::Math::Color& a_set) { m_baseColor = a_set; }

        void SetBaseColorTextureSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_baseColorTextureSRVDescriptorIndex = a_set; }

    private:

        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

        FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(ModelStandardUnLitMaterialGPUData)
    };
}