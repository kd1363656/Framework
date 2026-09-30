#pragma once

class Application;

namespace Converter
{
    class ApplicationJsonConverter final
    {
    public:

         ApplicationJsonConverter() = default;
        ~ApplicationJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Application& a_application) const;

        nlohmann::json Serialize(const Application& a_application) const;

    private:

        static constexpr std::string_view k_assetFilePathRegistryJsonKey = "AssetFilePathRegistry";
    };
}