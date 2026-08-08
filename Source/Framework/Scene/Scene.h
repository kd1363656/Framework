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

		void SetIsGameObjectExecutionLevelListDirty(const bool a_set) { m_isGameObjectExecutionLevelListDirty = a_set; }

		std::weak_ptr<GameObject> FindVALGameObject(const UUID& a_uuid) const;

		const auto& GetREFGameObjectList() const { return m_gameObjectList; }

		const auto& GetREFPrefabSystem() const { return m_prefabSystem; }

		auto& GetMutableREFPrefabSystem() { return m_prefabSystem; }

	private:

		void RemoveDestroyedGameObjects();

		void RefreshGameObjectExecutionLevelListIfNeeded();

		void RebuildGameObjectExecutionLevelList();

		void CalculateGameObjectExecutionLevel(const std::weak_ptr<GameObject>& a_gameObject, std::size_t& a_executionLevel) const;

		void AddGameObjectToExecutionLevelList(const std::weak_ptr<GameObject>& a_gameObject, const std::size_t& a_executionLevel);
		
		static constexpr std::size_t k_initialExecutionLevel = 0ULL;;

		std::vector<std::shared_ptr<GameObject>>            m_gameObjectList               = {};
		std::vector<std::vector<std::weak_ptr<GameObject>>> m_gameObjectExecutionLevelList = {};

		UUIDRegistry<std::weak_ptr<GameObject>> m_gameObjectUUIDRegistry = {};

		PrefabSystem m_prefabSystem = {};

		Converter::SceneJsonConverter m_jsonConverter = {};

		Graphics::LightSystem m_lightSystem = {};

		bool m_isGameObjectExecutionLevelListDirty = false;
	};
}