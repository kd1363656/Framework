#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	private:

		using SceneName     = std::string;
		using SceneFilePath = std::string;

		friend class SingletonBase<SceneManager>;

		 SceneManager()          = default;
		~SceneManager() override = default;

	public:

		void INIT	      ();
		void LoadScene    ();
		void PostLoadScene() const;

		void EarlyUpdate();
		void PreUpdate  ();
		void Update     ();
		void LateUpdate () const;
		void FixMatrix  ();
		
		void SaveScene () const;

	private:

		std::unordered_map<SceneName, SceneFilePath> m_sceneShiftSet = {};

		Scene m_scene = {};

		SceneName m_nextSceneName = {};
	};
}