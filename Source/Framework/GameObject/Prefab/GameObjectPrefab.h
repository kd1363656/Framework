#pragma once

namespace FWK
{
    class SceneGameObjectPrefabSystem;
}

namespace FWK
{
    class GameObjectPrefab final
    {
    public:

         GameObjectPrefab() = default;
        ~GameObjectPrefab() = default;

        void Load(const std::filesystem::path& a_filePath);

        bool Save(const std::filesystem::path&       a_filePath, 
                  const boost::uuids::uuid&          a_prefabUUID,
                        SceneGameObjectPrefabSystem& a_prefabSystem, 
                        GameObject&                  a_gameObject);

        void SetJson(nlohmann::json&& a_set) { m_json = std::move(a_set); }

        void SetName(const std::string& a_set) { m_name = a_set; }

        const auto& GetREFJson() const { return m_json; }

        const auto& GetREFName() const { return m_name; }

    private:

        nlohmann::json m_json = {};

        std::string m_name = {};

        Converter::GameObjectPrefabJsonConverter m_jsonConverter = {};
    };
}