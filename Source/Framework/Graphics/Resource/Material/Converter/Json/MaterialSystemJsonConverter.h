#pragma once

namespace FWK::Graphics
{
    class MaterialSystem;
}

namespace FWK::Converter
{
    class MaterialSystemJsonConverter final
    {
    public:

         MaterialSystemJsonConverter() = default;
        ~MaterialSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::MaterialSystem& a_materialSystem) const;

        nlohmann::json Serialize(const Graphics::MaterialSystem& a_materialSystem) const;

    private:

        static constexpr std::string_view k_materialStorageJsonKey = "MaterialStorage";
    };
}