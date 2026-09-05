#pragma once

namespace FWK
{
	class SceneManager;
}

namespace FWK::Converter
{
	class SceneManagerJsonConverter
	{
	public:

		 SceneManagerJsonConverter() = default;
		~SceneManagerJsonConverter() = default;

		void Load(SceneManager& a_sceneManager) const;

		void Save(const SceneManager& a_sceneManager) const;

	private:

		void DeserializeNextSceneLoadFilePathMap(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const;

		nlohmann::json SerializeNextSceneLoadFilePathMap(const SceneManager& a_sceneManager) const;

		static constexpr std::string_view k_assetFilePathRegistryJsonKey    = "AssetFilePathRegistry";
		static constexpr std::string_view k_nextSceneLoadFilePathMapJsonKey = "NextSceneLoadFilePathMap";
		static constexpr std::string_view k_sceneUUIDJsonKey                = "SceneUUID";
		static constexpr std::string_view k_nextSceneLoadFilePathJsonKey    = "NextSceneFilePath";
		static constexpr std::string_view k_sceneJsonKey                    = "Scene";
		static constexpr std::string_view k_sceneShiftEventObserverJsonKey  = "SceneShiftEventObserver";
	};
}