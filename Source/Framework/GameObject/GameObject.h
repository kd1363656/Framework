#pragma once

namespace FWK
{
	class Scene;
	class TransformComponent;
}

namespace FWK
{
	class GameObject final : public std::enable_shared_from_this<GameObject>
	{
	public:

		 GameObject() = default;
		~GameObject() = default;

		void INIT();

		void Deserialize(const nlohmann::json& a_rootJson, std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet, Scene& a_scene);

		bool DeserializePrefab(const nlohmann::json&                                                   a_rootJson, 
			                         std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
			                         Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
			                         std::unordered_set<boost::uuids::uuid>&                           a_parentPrefabUUIDSet,
			                         Scene&                                                            a_scene);

		bool DeserializePrefabInstance(const nlohmann::json& a_prefabJson, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene);

		bool DeserializeScene(const nlohmann::json&                                                   a_rootJson,
			                        std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
			                        Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
			                        Scene&                                                            a_scene);

		void RecursiveAddComponent(const Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList);
		bool RecursiveAddChild    (      std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,         Scene&                                     a_scene);
		
		void PostDeserialize();

		void EarlyUpdate   () const;
		void Update        () const;
		void LateUpdate    () const;
		void PostLateUpdate() const;

		void Destroy();

		void EditInspector();

		nlohmann::json SerializeScene () const;
		nlohmann::json SerializePrefab() const;

		void AddComponent   (const std::shared_ptr<ComponentBase>& a_component);
		void RemoveComponent(const std::weak_ptr<ComponentBase>&   a_component);

		bool ApplyParent(const std::weak_ptr<GameObject>& a_child);
		void Unparent   (const std::weak_ptr<GameObject>& a_child);

		void SetParent(const std::weak_ptr<GameObject>& a_set) { m_parent = a_set; }

		void SetComponentEventObserver(const std::shared_ptr<Observer<Enum::ComponentEvent>>& a_set) { m_componentEventObserver = a_set; }

		void SetPrefabUUID       (const boost::uuids::uuid& a_set) { m_prefabUUID        = a_set; }
		void SetSceneInstanceUUID(const boost::uuids::uuid& a_set) { m_sceneInstanceUUID = a_set; }

		void SetPrefabSceneInstanceNUM(const TypeAlias::PrefabSceneInstanceNUM a_set) { m_prefabSceneInstanceNUM = a_set; }

		void SetSceneInstanceName(const std::string& a_set) { m_sceneInstanceName = a_set; }

		template <Concept::IsDerivedComponentBaseConcept ComponentType>
		std::weak_ptr<ComponentType> FindUniqueComponent() const
		{
			const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

			if (auto l_itr = m_uniqueComponentMap.find(l_staticTypeID);
				l_itr != m_uniqueComponentMap.end())
			{
				if (auto l_component = l_itr->second.lock())
				{
					return std::static_pointer_cast<ComponentType>(l_component);
				}
			}
	
			return std::weak_ptr<ComponentType>();
		}

		template <Concept::IsDerivedComponentBaseConcept ComponentType>
		std::vector<std::weak_ptr<ComponentType>> FindMultiComponent() const
		{
			const auto l_staticTypeID = ComponentType::GetREFTypeINFO().k_staticTypeID;

			std::vector<std::weak_ptr<ComponentType>> l_list = {};

			if (auto l_itr = m_multiComponentMap.find(l_staticTypeID);
				l_itr != m_multiComponentMap.end())
			{
				l_list.reserve(l_itr->second.size());

				for (const auto& l_commponent : l_itr->second)
				{
					l_list.emplace_back(l_commponent);
				}
			}
	
			return l_list;
		}

		std::string FetchVALGameObjectName() const;

		const auto& GetREFParent() const { return m_parent; }

		const auto& GetREFChildSmartPointerVectorArray    () const { return m_childSmartPointerVectorArray; }
		const auto& GetREFComponentSmartPointerVectorArray() const { return m_componentSmartPointerVectorArray; }

		const std::weak_ptr<Observer<Enum::ComponentEvent>> GetVALComponentEventObserver() const { return m_componentEventObserver; }

		const auto& GetREFPrefabUUID       () const { return m_prefabUUID; }
		const auto& GetREFSceneInstanceUUID() const { return m_sceneInstanceUUID; }

		const auto& GetREFSceneInstanceName() const { return m_sceneInstanceName; }

		auto& GetMutableREFParent() { return m_parent; }

		auto& GetMutableREFSceneInstanceUUID() { return m_sceneInstanceUUID; }

		auto& GetMutableREFPrefabUUID            () { return m_prefabUUID; }
		auto& GetMutableREFComponentEventObserver() { return m_componentEventObserver; }

		std::weak_ptr<TransformComponent> GetVALTransformComponent() const { return m_transformComponent; }

		auto GetVALPrefabSceneInstanceNUM() const { return m_prefabSceneInstanceNUM; }

		bool GetVALIsDestroyed() const { return m_isDestroyed; }

	private:

		bool ContainsDuplicatePrefabUUIDRecursive(const std::weak_ptr<GameObject>& a_gameObject, std::unordered_set<boost::uuids::uuid>& a_parentPrefabUUIDSet) const;

		bool IsDescendantOf(const std::shared_ptr<GameObject>& a_ancestor) const;

		std::unordered_map<std::uint32_t, std::weak_ptr<ComponentBase>>				 m_uniqueComponentMap = {};
		std::unordered_map<std::uint32_t, std::vector<std::weak_ptr<ComponentBase>>> m_multiComponentMap  = {};

		std::shared_ptr<TransformComponent> m_transformComponent = std::make_shared<TransformComponent>();

		std::shared_ptr<Observer<Enum::ComponentEvent>> m_componentEventObserver = nullptr;

		std::weak_ptr<GameObject> m_parent = {};

		Utility::SmartPointerVectorArray<std::weak_ptr<GameObject>>      m_childSmartPointerVectorArray     = {};
		Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> m_componentSmartPointerVectorArray = {};

		Converter::GameObjectJsonConverter m_jsonConverter = {};

		boost::uuids::uuid m_prefabUUID        = {};
		boost::uuids::uuid m_sceneInstanceUUID = {};

		TypeAlias::PrefabSceneInstanceNUM m_prefabSceneInstanceNUM = Constant::k_invalidPrefabSceneInstanceNUM;

		std::string m_sceneInstanceName = {};

		bool m_isDestroyed = false;
	};
}