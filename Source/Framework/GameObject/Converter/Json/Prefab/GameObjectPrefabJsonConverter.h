#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK::Converter
{
	class GameObjectPrefabJsonConverter final
	{
	public:

		 GameObjectPrefabJsonConverter() = default;
		~GameObjectPrefabJsonConverter() = default;

		bool Deserialize(const std::weak_ptr<GameObject>&                                        a_gameObject, 
			             const nlohmann::json&                                                   a_rootJson,
	                           std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                           std::unordered_set<boost::uuids::uuid>&                           a_prefabUUIDSet, 
	                           Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                           Scene&                                                            a_scene) const;

		nlohmann::json Serialize(const GameObject& a_gameObject) const;

	private:

		bool DeserializePrefabComponent             (const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray) const;
		bool DeserializePrefabComponentEventObserver(const nlohmann::json&            a_rootJson,   GameObject&           a_gameObject)                                                                                                     const;

		bool DeserializePrefabChildList(const nlohmann::json&                            a_rootJsonArray,
			                                  std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet, 
			                                  std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
			                                  Scene&                                     a_scene) const;

		nlohmann::json SerializePrefabComponent        (const GameObject& a_gameObject) const;
		nlohmann::json SerializePrefabComponentObserver(const GameObject& a_gameObject) const;
		nlohmann::json SerializePrefabChildList        (const GameObject& a_gameObject) const;

		static constexpr std::string_view k_componentEventObserverJsonKey = "ComponentEventObserver";
	};
}