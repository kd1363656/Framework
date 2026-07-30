#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK
{
	class GameObjectJsonConverter final
	{
	public:

		 GameObjectJsonConverter() = default;
		~GameObjectJsonConverter() = default;

		void Deserialize      (const nlohmann::json& a_rootJson, GameObject&                                  a_gameObject)                                                                       const;
		void DeserializePrefab(const nlohmann::json& a_rootJson, std::vector<std::shared_ptr<ComponentBase>>& a_componentList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeData) const;
		void DeserializeScene (const nlohmann::json& a_rootJson, std::vector<std::shared_ptr<ComponentBase>>& a_componentList, std::vector<Struct::ChildDeserializeData>& a_childDeserializeData) const;

		nlohmann::json SerializePrefab(const GameObject& a_gameObject) const;
		nlohmann::json SerializeScene (const GameObject& a_gameObject) const;

	private:

		static constexpr std::string_view k_prefabNameJsonKey = "PrefabName";
	};
}