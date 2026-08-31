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

		void Deserialize(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const;

		nlohmann::json Serialize(SceneManager& a_sceneManager) const;

	private:

		void DeserializeSceneShiftMap(const nlohmann::json& a_rootJson, SceneManager& a_sceneManager) const;

		nlohmann::json SerializeSceneShiftMap(const SceneManager& a_sceneManager) const;

		static constexpr std::string_view k_sceneUUIDJsonKey     = "SceneUUID";
		static constexpr std::string_view k_sceneFilePathJsonKey = "SceneFilePath";
		static constexpr std::string_view k_sceneShiftMapJsonKey = "SceneShiftMap";
		static constexpr std::string_view k_sceneJsonKey         = "Scene";
	};
}