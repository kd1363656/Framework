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

		void LoadScene(const std::string_view& a_sceneFilePath);
		
		void EarlyUpdate   ();
		void Update        () const;
		void LateUpdate    () const;
		void PostLateUpdate() const;
		
		void SaveScene();

		void AddSceneShiftMap(const SceneName& a_sceneName, const SceneFilePath& a_sceneFilePath);

		const auto& GetREFSceneShiftMap() const { return m_sceneShiftMap; }

		std::weak_ptr<Scene> GetVALScene() const { return m_scene; }

	private:

		void INIT();

		void SceneShiftIfNeeded();

		std::string FindSceneShiftFilePath(const std::string_view& a_sceneName) const;

		SceneShiftMap m_sceneShiftMap = {};

		std::shared_ptr<Scene> m_scene = nullptr;

		Converter::SceneManagerJsonConverter m_jsonConverter = {};

		SceneName     m_nextSceneName        = {};
		SceneFilePath m_currentSceneFilePath = {};
	};
}