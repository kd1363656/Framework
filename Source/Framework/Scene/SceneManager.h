#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	private:

		using NextSceneLoadFilePathMap = std::unordered_map<boost::uuids::uuid, std::filesystem::path>;

		friend class SingletonBase<SceneManager>;

		 SceneManager()          = default;
		~SceneManager() override = default;

	public:

		void LoadScene(const std::filesystem::path& a_nextSceneLoadFilePath);
		
		void EarlyUpdate   ();
		void Update        () const;
		void LateUpdate    () const;
		void PostLateUpdate();
		
		void SaveScene() const;
		bool SaveScene(const std::filesystem::path& a_nextSceneLoadFilePath);
		
		void AddNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID, const std::filesystem::path& a_nextSceneLoadFilePath);

		const auto& GetREFNextSceneLoadFilePathMap() const { return m_nextSceneLoadFilePathMap; }
		
		const auto& GetREFSceneShiftEventObserver() const { return m_sceneShiftEventObserver; }

		auto& GetMutableREFSceneShiftEventObserver() { return m_sceneShiftEventObserver; }

		std::weak_ptr<Scene> GetVALScene() const { return m_scene; }

	private:

		void INIT();

		void LoadNextSceneIfNeeded();

		const std::filesystem::path* FindPTRNextSceneLoadFilePath(const boost::uuids::uuid& a_uuid) const;

		NextSceneLoadFilePathMap m_nextSceneLoadFilePathMap = {};

		std::shared_ptr<Scene> m_scene = nullptr;

		Observer<Enum::SceneShiftEvent> m_sceneShiftEventObserver = {};

		Converter::SceneManagerJsonConverter m_jsonConverter = {};

		std::filesystem::path m_currentSceneFilePath = {};

		boost::uuids::uuid m_nextSceneUUID = {};
	};
}