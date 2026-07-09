#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	private:

		using SceneName     = std::string;
		using SceneFilePath = std::string;
		using SceneShiftMap = std::unordered_map<SceneName, SceneFilePath, TypeINFORegistry::StringHash, std::equal_to<>>;

		friend class SingletonBase<SceneManager>;

		 SceneManager()          = default;
		~SceneManager() override = default;

	public:

		void LoadScene(const std::string_view& a_sceneFilePath);
		
		void EarlyUpdate  ();
		void Update       ();
		void LateUpdate   () const;
		void ConfrimMatrix();
		
		void SaveScene() const;

		void AddSceneShiftMap(const SceneName& a_sceneName, const SceneFilePath& a_sceneFilePath);

		const auto& GetREFSceneShiftMap() const { return m_sceneShiftMap; }

		const auto& GetREFScene() const { return m_scene; }

		auto& GetMutableREFScene() { return m_scene; }

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