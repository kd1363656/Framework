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
			                   std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet,
			                   Scene&                                  a_scene) const;
		
		bool DeserializePrefab(const std::weak_ptr<GameObject>&                                        a_gameObject, 
			                   const nlohmann::json&                                                   a_rootJson,
	                                 std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                 std::unordered_set<boost::uuids::uuid>&                           a_prefabUUIDSet, 
	                                 Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                 Scene&                                                            a_scene) const;

	private:

		bool DeserializePrefabComponent             (const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray) const;
		bool DeserializePrefabComponentEventObserver(const nlohmann::json&            a_rootJson,   GameObject&           a_gameObject)                                                                                                     const;

		bool DeserializePrefabChildList(const nlohmann::json&                            a_rootJsonArray,
			                                  std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet, 
			                                  std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
			                                  Scene&                                     a_scene) const;

		static constexpr std::string_view k_prefabUUIDJsonKey             = "PrefabUUID";
		static constexpr std::string_view k_transformComponentJsonKey     = "TransformComponent";
		static constexpr std::string_view k_componentListJsonKey          = "ComponentList";
		static constexpr std::string_view k_componentTypeNameJsonKey      = "ComponentTypeName";
		static constexpr std::string_view k_componentEventObserverJsonKey = "ComponentEventObserver";
		static constexpr std::string_view k_childListJsonKey              = "ChildList";
	};
}