#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	private:

		using SceneName     = std::string;
		using SceneFilePath = std::string;
		using SceneShiftMap = std::unordered_map<SceneName, SceneFilePath, Struct::StringHash, std::equal_to<>>;

		friend class SingletonBase<SceneManager>;

		 SceneManager()          = default;
		~SceneManager() override = default;

	public:

		void LoadScene    (const std::string_view& a_sceneFilePath);
		void PostLoadScene() const;

		void EarlyUpdate();
		void PreUpdate  ();
		void Update     ();
		void LateUpdate () const;
		void FixMatrix  ();
		
		void SaveScene () const;

		void AddSceneShiftMap(const SceneName& a_sceneName, const SceneFilePath& a_sceneFilePath);

	private:

		void INIT();

		void SceneShiftIfNeeded();

		std::string FindSceneShiftFilePath(const std::string_view& a_sceneName) const;

		SceneShiftMap m_sceneShiftMap = {};

		Scene m_scene = {};

		Converter::SceneManagerJsonConverter m_jsonConverter = {};

		SceneName     m_nextSceneName        = {};
		SceneFilePath m_currentSceneFilePath = {};
	};
}