#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK::Converter
{
	class GameObjectSceneJsonConverter final
	{
	public:

		 GameObjectSceneJsonConverter() = default;
		~GameObjectSceneJsonConverter() = default;

		bool Deserialize(const nlohmann::json&                                                   a_rootJson, 
			                   std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
			                   Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
			                   GameObject&                                                       a_gameObject,
			                   Scene&                                                            a_scene) const;

		nlohmann::json Serialize(const GameObject& a_gameObject) const;

	private:

		bool DeserializeSceneComponent(const nlohmann::json& a_rootJson,      Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, GameObject& a_gameObject) const;
		bool DeserializeSceneChildList(const nlohmann::json& a_rootJsonArray, std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,         Scene&      a_scene)      const;

		nlohmann::json SerializeSceneComponent(const GameObject& a_gameObject) const;
		nlohmann::json SerializeSceneChildList(const GameObject& a_gameObject) const;

		static constexpr std::string_view k_sceneInstanceNameJsonKey      = "SceneInstanceName";
		static constexpr std::string_view k_prefabSceneInstanceNUMJsonKey = "PrefabSceneInstanceNUM";
		static constexpr std::string_view k_sceneInstanceUUIDJsonKey      = "SceneInstanceUUID";
	};
}