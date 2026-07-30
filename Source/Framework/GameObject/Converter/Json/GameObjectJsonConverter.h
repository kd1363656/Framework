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

		void Deserialize      (const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Scene& a_scene) const;
		void DeserializePrefab(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Scene& a_scene) const;

		void DeserializePrefab(const nlohmann::json&                                                   a_rootJson,
			                         std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeData, 
			                         Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
			                         Scene&                                                            a_scene) const;

		void DeserializeScene(const nlohmann::json&                                                   a_rootJson, 
			                        std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeData, 
			                        Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
			                        Scene&                                                            a_scene) const;

		nlohmann::json SerializePrefab(const GameObject& a_gameObject) const;
		nlohmann::json SerializeScene (const GameObject& a_gameObject) const;

	private:

		void RecursiveAddComponent(const std::shared_ptr<GameObject>&                                      a_self,   
			                             std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeData,
			                             Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
			                             Scene&                                                            a_scene) const;

		void RecursiveAddChild(const std::shared_ptr<GameObject>& a_parent, std::vector<Struct::ChildDeserializeData>& a_childDeserializeData, Scene& a_scene) const;

		static constexpr std::string_view k_prefabNameJsonKey               = "PrefabName";
		static constexpr std::string_view k_contanisNumberPrefabNameJsonKey = "ContainsNumberPrefabName";
	};
}