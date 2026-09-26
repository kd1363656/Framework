#pragma once

namespace FWK
{
    class SceneChanger;
}

namespace FWK::Converter
{
    class SceneChangerJsonConverter final
    {
    public:

         SceneChangerJsonConverter() = default;
        ~SceneChangerJsonConverter() = default;
    
        void Deserialize(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneChanger& a_sceneChanger) const;

        nlohmann::json Serialize(const AssetFilePathRegistry& a_assetFilePathRegistry, SceneChanger& a_sceneChanger) const;

    private:

        void DeserializeNextSceneMap(const nlohmann::json& a_rootJson, const AssetFilePathRegistry& a_assetFilePathRegistry, SceneChanger& a_sceneChanger) const;

        nlohmann::json SerializeNextSceneMap(const AssetFilePathRegistry& a_assetFilePathRegistry, SceneChanger& a_sceneChanger) const;

        static constexpr std::string_view k_sceneChangeEventObserverJsonKey = "SceneChangeEventObserver";
        static constexpr std::string_view k_nextSceneMapJsonKey             = "NextSceneMap";
        static constexpr std::string_view k_uuidJsonKey                     = "UUID";
        static constexpr std::string_view k_nextSceneNameJsonKey            = "NextSceneName";
    };
}