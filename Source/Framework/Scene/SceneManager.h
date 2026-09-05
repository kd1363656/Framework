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
		
		bool AddNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID, const std::filesystem::path& a_nextSceneLoadFilePath);

		bool RemoveNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID);
		
		bool ReplaceSceneFilePath(const std::filesystem::path& a_oldSceneFilePath, const std::filesystem::path& a_newSceneFilePath, const boost::uuids::uuid& a_sceneUUID);

		const auto& GetREFNextSceneLoadFilePathMap() const { return m_nextSceneLoadFilePathMap; }
		
		const auto& GetREFSceneShiftEventObserver() const { return m_sceneShiftEventObserver; }

		const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

		const auto& GetREFCurrentSceneFilePath() const { return m_currentSceneFilePath; }

		auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

		auto& GetMutableREFSceneShiftEventObserver() { return m_sceneShiftEventObserver; }

		std::weak_ptr<Scene> GetVALScene() const { return m_scene; }

	private:

		void INIT();

		void LoadNextSceneIfNeeded();

		NextSceneLoadFilePathMap m_nextSceneLoadFilePathMap = {};

		std::shared_ptr<Scene> m_scene = nullptr;

		Observer<Enum::SceneShiftEvent> m_sceneShiftEventObserver = {};

		AssetFilePathRegistry m_assetFilePathRegistry = {};

		Converter::SceneManagerJsonConverter m_jsonConverter = {};

		std::filesystem::path m_currentSceneFilePath = {};

		boost::uuids::uuid m_nextSceneUUID = {};
	};
}