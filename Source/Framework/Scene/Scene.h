#pragma once

namespace FWK
{
	class Scene final
	{
	public:

		 Scene() = default;
		~Scene() = default;

		void INIT		    ();
		void Deserialize    (const nlohmann::json& a_rootJson);
		void PostDeserialize() const;

		void EarlyUpdate   ();
		void Update        () const;
		void LateUpdate    () const;
		void PostLateUpdate() const;

		nlohmann::json Serialize();

		void AddGameObject(const std::shared_ptr<GameObject>& a_gameObject);

		void SetSceneName(const std::string& a_set) { m_sceneName = a_set; }

		void SetIsGameObjectExecutionLevelListDirty(const bool a_set) { m_isGameObjectExecutionLevelListDirty = a_set; }

		std::weak_ptr<GameObject> FindVALGameObject(const boost::uuids::uuid& a_uuid) const;

		const auto& GetREFGameObjectList() const { return m_gameObjectList; }

		const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

		const auto& GetREFPrefabSystem() const { return m_prefabSystem; }

		const auto& GetREFSceneName() const { return m_sceneName; }

		auto& GetMutableREFPrefabSystem() { return m_prefabSystem; }

		auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

	private:

		void RemoveDestroyedGameObjects();

		void RefreshGameObjectExecutionLevelListIfNeeded();

		void RebuildGameObjectExecutionLevelList();

		void CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject, std::size_t& a_executionLevel) const;

		void AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel);
		
		static constexpr std::size_t k_initialExecutionLevel = 0ULL;

		std::vector<std::shared_ptr<GameObject>>            m_gameObjectList               = {};
		std::vector<std::vector<std::weak_ptr<GameObject>>> m_gameObjectExecutionLevelList = {};

		UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

		AssetFilePathRegistry m_assetFilePathRegistry = {};

		PrefabSystem m_prefabSystem = {};

		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};

		std::string m_sceneName = {};

		bool m_isGameObjectExecutionLevelListDirty = false;
	};
}