#pragma once

namespace FWK::Graphics
{
    class ModelStandardLitMaterialGPUData final
    {
    public:

         ModelStandardLitMaterialGPUData() = default;
        ~ModelStandardLitMaterialGPUData() = default;

        void SetBaseColor(const TypeAlias::Math::Color& a_set) { m_baseColor = a_set; }

        void SetMetallic (const float a_set) { m_metallic  = a_set; }
        void SetRoughness(const float a_set) { m_roughness = a_set; }

        void SetBaseColorTextureSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_baseColorTextureSRVDescriptorIndex = a_set; }
        void SetNormalTextureSRVDescriptorIndex   (const TypeAlias::DescriptorIndex a_set) { m_normalTextureSRVDescriptorIndex    = a_set; }
        void SetMetallicTextureSRVDescriptorIndex (const TypeAlias::DescriptorIndex a_set) { m_metallicTextureSRVDescriptorIndex  = a_set; }
        void SetRoughnessTextureSRVDescriptorIndex(const TypeAlias::DescriptorIndex a_set) { m_roughnessTextureSRVDescriptorIndex = a_set; }

        static constexpr float k_defaultMetallic  = 0.0F;
        static constexpr float k_defaultRoughness = 1.0F;

    private:

        TypeAlias::Math::Color m_baseColor = Constant::k_whiteColor;

        float m_metallic  = k_defaultMetallic;
        float m_roughness = k_defaultRoughness;

        TypeAlias::DescriptorIndex m_baseColorTextureSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_normalTextureSRVDescriptorIndex    = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_metallicTextureSRVDescriptorIndex  = DescriptorHeap::k_invalidDescriptorIndex;
        TypeAlias::DescriptorIndex m_roughnessTextureSRVDescriptorIndex = DescriptorHeap::k_invalidDescriptorIndex;

        FWK_DEFINE_MODEL_MATERIAL_RENDER_TABLE_INFO(ModelStandardLitMaterialGPUData)
    };
}