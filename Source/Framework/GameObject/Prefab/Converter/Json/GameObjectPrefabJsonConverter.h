#pragma once

namespace FWK
{
    class GameObjectPrefab;
    class SceneGameObjectPrefabSystem;
}

namespace FWK::Converter
{
    class GameObjectPrefabJsonConverter final
    {
    public:

         GameObjectPrefabJsonConverter() = default;
        ~GameObjectPrefabJsonConverter() = default;

        static bool Rename(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const std::string& a_newName);

        void Load(const nlohmann::json& a_rootJson, GameObjectPrefab& a_gameObejctPrefab) const;

        bool Save(const std::filesystem::path&       a_filePath, 
                  const GameObject&                  a_gameObject, 
                        SceneGameObjectPrefabSystem& a_prefabSystem,
                        GameObjectPrefab&            a_prefab) const;

    private:

        static constexpr std::string_view k_prefabJsonKey = "Prefab";
        static constexpr std::string_view k_nameJsonKey   = "Name";
    };
}