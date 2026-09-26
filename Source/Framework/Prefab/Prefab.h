#pragma once

namespace FWK
{
    class Prefab final
    {
    public:

         Prefab() = default;
        ~Prefab() = default;

        void Load(const std::filesystem::path& a_filePath);

        bool Save(const std::filesystem::path& a_filePath, const GameObject& a_gameObject);

        void SetJson(const nlohmann::json& a_set) { m_json = a_set; }

        void SetPrefabName(const std::string& a_set) { m_prefabName = a_set; }

        const auto& GetREFJson() const { return m_json; }

        const auto& GetREFPrefabName() const { return m_prefabName; }

        auto& GetMutableREFJson() { return m_json; }

    private:

        Converter::PrefabJsonConverter m_jsonConverter = {};

        nlohmann::json m_json = {};

        std::string m_prefabName = {};
    };
}