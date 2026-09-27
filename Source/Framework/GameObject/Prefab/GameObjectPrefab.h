#pragma once

namespace FWK
{
    class GameObjectPrefab final
    {
    public:

         GameObjectPrefab() = default;
        ~GameObjectPrefab() = default;

        void Load(const std::filesystem::path& a_filePath);

        bool Save(const std::filesystem::path& a_filePath, const GameObject& a_gameObject);

        void SetJson(const nlohmann::json& a_set) { m_json = a_set; }

        void SetName(const std::string& a_set) { m_name = a_set; }

        const auto& GetREFJson() const { return m_json; }

        const auto& GetREFName() const { return m_name; }

        auto& GetMutableREFJson() { return m_json; }

    private:

        Converter::GameObjectPrefabJsonConverter m_jsonConverter = {};

        nlohmann::json m_json = {};

        std::string m_name = {};
    };
}