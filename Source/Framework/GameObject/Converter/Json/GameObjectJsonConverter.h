#pragma once

namespace FWK
{
	class GameObject;
	class Scene;
}

namespace FWK
{
	class GameObjectJsonConverter final
	{
	public:

		 GameObjectJsonConverter() = default;
		~GameObjectJsonConverter() = default;

		void Deserialize(const std::weak_ptr<GameObject>&              a_gameObject,
			             const nlohmann::json&                         a_rootJson, 
			                   std::unordered_set<boost::uuids::uuid>& a_parentPrefabUUIDSet, 
			                   Scene&                                  a_scene) const;

		void DeserializePrefab(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Scene&                                  a_scene) const;

		// a_rootJsonがそもそもプレハブのjsonを読み込む前提
		void DeserializePrefab(const nlohmann::json&                                                   a_rootJson,
			                   const std::weak_ptr<GameObject>&                                        a_gameObject,
			                         std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
			                         Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
			                         std::unordered_set<boost::uuids::uuid>&                           a_parentPrefabUUIDSet,
			                         Scene&                                                            a_scene) const;

		void DeserializeScene(const nlohmann::json&                                                   a_rootJson, 
			                        GameObject&                                                       a_gameObject,
			                        std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
			                        Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
			                        Scene&                                                            a_scene) const;

		nlohmann::json SerializePrefab(const GameObject& a_gameObject) const;
		nlohmann::json SerializeScene (const GameObject& a_gameObject) const;

	private:

		void DeserializeComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const;

		void DeserializeChildPrefab(const nlohmann::json&                            a_rootJson, 
			                              std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
			                              std::unordered_set<boost::uuids::uuid>&    a_parentPrefabNameSet,
			                              Scene&                                     a_scene) const;

		void DeserializeChildScene (const nlohmann::json& a_rootJson, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene) const;

		nlohmann::json SerializeComponentObserver(const GameObject& a_gameObject) const;
		nlohmann::json SerializeChildPrefab      (const GameObject& a_gameObject) const;
		nlohmann::json SerializeChildScene       (const GameObject& a_gameObject) const;

		void RecursiveAddComponent(const std::shared_ptr<GameObject>&                                      a_self,   
			                       const Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
			                             std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
			                             Scene&                                                            a_scene) const;

		void RecursiveAddChild(const std::shared_ptr<GameObject>&               a_parent, 
			                         std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
			                         std::unordered_set<boost::uuids::uuid>&    a_parentPrefabNameSet, 
			                         Scene&                                     a_scene) const;

		static constexpr std::string_view k_sceneInstanceNameJsonKey        = "SceneInstanceName";
		static constexpr std::string_view k_prefabUUIDJsonKey               = "PrefabUUID";
		static constexpr std::string_view k_sceneInstanceUUIDJsonKey        = "SceneInstanceUUID";
		static constexpr std::string_view k_prefabSceneInstanceNUMJsonKey   = "PrefabSceneInstanceNUM";
		static constexpr std::string_view k_componentListJsonKey            = "ComponentList";
		static constexpr std::string_view k_typeNameJsonKey                 = "TypeName";
		static constexpr std::string_view k_componentEventObserverJsonKey   = "ComponentEventObserver";
		static constexpr std::string_view k_childListJsonKey                = "ChildList";
		static constexpr std::string_view k_transformComponentJsonKey       = "TransformComponent";
	};
}