#pragma once

namespace FWK
{
	class GameObject;
	class Scene;
}

namespace FWK::Converter
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

		bool DeserializePrefabInstance(const std::weak_ptr<GameObject>&                 a_gameObject, 
			                           const nlohmann::json&                            a_prefabJson,
	                                         std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
			                                 Scene&                                     a_scene) const;

		bool DeserializeScene(const nlohmann::json&                                                   a_rootJson, 
			                        std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
			                        Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
			                        GameObject&                                                       a_gameObject,
			                        Scene&                                                            a_scene) const;

		nlohmann::json SerializePrefab(const GameObject& a_gameObject) const;
		nlohmann::json SerializeScene (const GameObject& a_gameObject) const;

	private:

		GameObjectPrefabJsonConverter m_prefabJsonConverter = {};
		GameObjectSceneJsonConverter  m_sceneJsonConverter  = {};
	};
}