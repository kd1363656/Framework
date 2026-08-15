#include "GameObjectJsonConverter.h"

void FWK::Converter::GameObjectJsonConverter::Deserialize(const std::weak_ptr<GameObject>&              a_gameObject, 
	                                                      const nlohmann::json&                         a_rootJson,  
	                                                            std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet,
	                                                            Scene&                                  a_scene) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG("ゲームオブジェクトが無効になっておりデシリアライズ処理に失敗しました。");

		return;
	}

	// コンポーネント、ゲームオブジェクトをPrefabをデシリアライズしてからSceneのデシリアライズを行う際に
	// 必要な子やコンポーネントの情報を保持する
	Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> l_componentLoadVectorArray = {};
	std::vector<Struct::ChildDeserializeData>                        l_childLoadList            = {};

	// まずはプレハブからデシリアライズ
	// ここでプレハブ識別UUIDや子などのシーンに配置する
	// ゲームオブジェクトの親子構造やコンポーネントなどの型を構築してしまう
	// プレハブを前提にコンポーネントや親子関係を構築するのでプレハブのデシリアライズに失敗したらreturn
	if (!DeserializePrefab(a_gameObject,
		                   a_rootJson,
		                   l_childLoadList,
		                   a_prefabUUIDSet,
		                   l_componentLoadVectorArray,
		                   a_scene))
	{
		return;
	}

	// デシリアライズし終わったコンポーネントや子ゲームオブジェクトにシーンデータを読み込ませる
	if (!DeserializeScene(a_rootJson,
	                      l_childLoadList,
	                      l_componentLoadVectorArray,
	                      *l_gameObject,
		                  a_scene))
	{
		return;
	}

	// デシリアライズした各コンポーネントを、親、子に再帰的に追加
	RecursiveAddComponent(l_gameObject,
		                  l_componentLoadVectorArray, 
		                  l_childLoadList, 
		                  a_scene);

	// 親子関係を再帰的に構築
	RecursiveAddChild(l_gameObject, l_childLoadList, a_scene);
}
bool FWK::Converter::GameObjectJsonConverter::DeserializePrefab(const std::weak_ptr<GameObject>&                                        a_gameObject,
	                                                            const nlohmann::json&                                                   a_rootJson,
	                                                                  std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                                  std::unordered_set<boost::uuids::uuid>&                           a_prefabUUIDSet, 
	                                                                  Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                                  Scene&                                                            a_scene) const
{
	return m_prefabJsonConverter.Deserialize(a_gameObject,
		                                     a_rootJson,
		                                     a_childDeserializeDataList,
		                                     a_prefabUUIDSet,
		                                     a_componentSmartPointerVectorArray,
		                                     a_scene);
}
bool FWK::Converter::GameObjectJsonConverter::DeserializeScene(const nlohmann::json&                                                   a_rootJson,
	                                                                 std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
	                                                                 Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                                                 GameObject&                                                       a_gameObject, 
	                                                                 Scene&                                                            a_scene) const
{
	return m_sceneJsonConverter.Deserialize(a_rootJson,
		                                    a_childDeserializeDataList,
		                                    a_componentSmartPointerVectorArray,
		                                    a_gameObject,
		                                    a_scene);
}

nlohmann::json FWK::Converter::GameObjectJsonConverter::SerializePrefab(const GameObject& a_gameObject) const
{
	return m_prefabJsonConverter.Serialize(a_gameObject);
}
nlohmann::json FWK::Converter::GameObjectJsonConverter::SerializeScene(const GameObject& a_gameObject) const
{
	return m_sceneJsonConverter.Serialize(a_gameObject);
}

void FWK::Converter::GameObjectJsonConverter::RecursiveAddComponent(const std::shared_ptr<GameObject>&                                      a_self,
	                                                     const Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                           std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                           Scene&                                                            a_scene) const
{
	if (!a_self) { return; }

	// 親のコンポーネントを親のコンポーネントリストに追加
	for (const auto& l_componentData : a_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		a_self->AddComponent(l_component);
	}

	// 子のコンポーネントを子のコンポーネントリストに追加
	for (auto& l_childLoad : a_childDeserializeDataList)
	{
		if (!l_childLoad.m_self) { continue; }

		// 子のコンポーネントも再帰的に追加していく
		RecursiveAddComponent(l_childLoad.m_self,
			                  l_childLoad.m_componentSmartPointerVectorArray,
			                  l_childLoad.m_childDeserializeDataList, 
			                  a_scene);
	}
}
void FWK::Converter::GameObjectJsonConverter::RecursiveAddChild(const std::shared_ptr<GameObject>& a_parent, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene) const
{
		// 子であろうが一つのリストに格納
	if (!a_parent) { return; }

	// 親子関係を再帰的に構築
	for (auto& l_childLoad : a_childDeserializeDataList)
	{
		const auto& l_child = l_childLoad.m_self;

		if (!l_child) { continue; }

		// 同じPrefab名が親経路に存在する場合や、
		// GameObjectの親子関係を構築できなかった場合は追加しない
		if (!a_parent->ApplyParent(l_child)) { continue; }

		// 親子関係を構築できたGameObjectだけをSceneへ追加する
		a_scene.AddGameObject(l_child);

		// 子のPrefabUUIDがSetへ入った状態で
		// 孫以下の親子関係を構築する
		RecursiveAddChild(l_child, l_childLoad.m_childDeserializeDataList, a_scene);
	}
}