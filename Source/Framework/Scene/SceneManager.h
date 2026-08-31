#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	private:

		using SceneFilePath = std::string;
		using SceneShiftMap = std::unordered_map<boost::uuids::uuid, SceneFilePath>;

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
		void SaveScene(const std::filesystem::path& a_sceneFilePath);

		void AddSceneShiftMap(const boost::uuids::uuid& a_sceneUUID, const SceneFilePath& a_sceneFilePath);

		void RequestSceneShift(const boost::uuids::uuid& a_sceneUUID);

		void SetCurrentSceneUUID(const boost::uuids::uuid& a_set) { m_currentSceneUUID = a_set; }

		const auto& GetREFSceneShiftMap() const { return m_sceneShiftMap; }

		const auto& GetREFCurrentSceneUUID() const { return m_currentSceneUUID; }

		std::weak_ptr<Scene> GetVALScene() const { return m_scene; }

	private:

		void INIT();

		void SceneShiftIfNeeded();

		SceneShiftMap m_sceneShiftMap = {};

		std::shared_ptr<Scene> m_scene = nullptr;

		Converter::SceneManagerJsonConverter m_jsonConverter = {};

		boost::uuids::uuid m_currentSceneUUID = {};
		boost::uuids::uuid m_nextSceneUUID    = {};
	};
}