#pragma once

namespace FWK
{
    class ComponentBase;
}

namespace FWK::Converter
{
    class ComponentBaseJsonConverter final
    {
    public:

         ComponentBaseJsonConverter() = default;
        ~ComponentBaseJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, ComponentBase& a_componentBase) const;
        
        nlohmann::json Serialize(const ComponentBase& a_componentBase) const;
        
    private:

        static constexpr std::string_view k_uuidJsonKey            = "UUID";
        static constexpr std::string_view k_isSerializeSkipJsonKey = "IsSerializeSkip";
        static constexpr std::string_view k_isDisableJsonKey       = "IsDisable";
    };
}