#pragma once

namespace FWK::Graphics
{
    class ModelRenderSystem;
}

namespace FWK::Converter
{
    class ModelRenderSystemJsonConverter final
    {
    public:

         ModelRenderSystemJsonConverter() = default;
        ~ModelRenderSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Graphics::ModelRenderSystem& a_modelRenderSystem) const;

        nlohmann::json Serialize(const Graphics::ModelRenderSystem& a_modelRenderSystem) const;

    private:

        static constexpr std::string_view k_tableSettingListJsonKey = "TableSettingList";
        static constexpr std::string_view k_typeJsonKey             = "Type";
        static constexpr std::string_view k_capacityJsonKey         = "Capacity";
    };
}