#pragma once

namespace FWK::Graphics
{
    class ModelMaterialSystem;
}

namespace FWK::Converter
{
    class ModelMaterialSystemJsonConverter final
    {
    public:

         ModelMaterialSystemJsonConverter() = default;
        ~ModelMaterialSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelMaterialSystem& a_modelMaterialSystem) const;

        nlohmann::json Serialize(const Graphics::ModelMaterialSystem& a_modelMaterialSystem) const;

    private:

        static constexpr std::string_view k_materialStorageJsonKey = "MaterialStorage";
    };
}