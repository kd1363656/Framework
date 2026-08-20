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
		FWK_ADD_LOG(Constant::k_debugWarningColor, "ゲームオブジェクトが無効になっておりデシリアライズ処理に失敗しました。");

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
	l_gameObject->RecursiveAddComponent(l_componentLoadVectorArray, l_childLoadList);

	// 親子関係を再帰的に構築
	l_gameObject->RecursiveAddChild(l_childLoadList, a_scene);
}
bool FWK::Converter::GameObjectJsonConverter::DeserializePrefab(const std::weak_ptr<GameObject>&                                        a_gameObject,
	                                                            const nlohmann::json&                                                   a_rootJson,
	                                                                  std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                                  std::unordered_set<boost::uuids::uuid>&                           a_prefabUUIDSet, 
	                                                                  Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                                  Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null()) { return false; }

	return m_prefabJsonConverter.Deserialize(a_gameObject,
		                                     a_rootJson,
		                                     a_childDeserializeDataList,
		                                     a_prefabUUIDSet,
		                                     a_componentSmartPointerVectorArray,
		                                     a_scene);
}
bool FWK::Converter::GameObjectJsonConverter::DeserializePrefabInstance(const std::weak_ptr<GameObject>&                 a_gameObject, 
	                                                                    const nlohmann::json&                            a_prefabJson, 
	                                                                          std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
	                                                                          Scene&                                     a_scene) const
{
	if (a_prefabJson.is_null()) { return false; }

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return false; }

	// Prefabの循環参照確認用
	std::unordered_set<boost::uuids::uuid> l_prefabUUIDSet = {};

	// RootGameObject自身のComponentを
	// PrefabJsonConverterが一旦ここへ生成する
	Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> l_componentSmartPointerVectorArray = {};

	// Prefab情報を生成
	if (!m_prefabJsonConverter.Deserialize(a_gameObject, 
		                                   a_prefabJson,
		                                   a_childDeserializeDataList,
		                                   l_prefabUUIDSet,
		                                   l_componentSmartPointerVectorArray,
		                                   a_scene))
	{
		return false;
	}

	// Component確定
	l_gameObject->RecursiveAddComponent(l_componentSmartPointerVectorArray, a_childDeserializeDataList);

	return true;
}
bool FWK::Converter::GameObjectJsonConverter::DeserializeScene(const nlohmann::json&                                                   a_rootJson,
	                                                                 std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
	                                                                 Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                                                 GameObject&                                                       a_gameObject, 
	                                                                 Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null()) { return false; }

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