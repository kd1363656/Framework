#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのデシリアライズに失敗しました。");
		return; 
	}

	const auto& l_scene        = SceneManager::GetInstance ().GetREFScene();
	const auto& l_prefabSystem = l_scene.GetREFPrefabSystem();
	const auto& l_gameObject   = a_gameObject.lock         ();

	// ゲームオブジェクトのプレハブ名を読み取る
	const auto& l_prefabName               = a_rootJson.value(k_prefabNameJsonKey,               std::string{});
	const auto& l_containsNumberPrefabName = a_rootJson.value(k_contanisNumberPrefabNameJsonKey, std::string{});

	if (l_prefabName.empty() ||
		l_containsNumberPrefabName.empty()) 
	{
		FWK_ADD_LOG("名前が空のプレハブがJsonファイルに含まれています。SceneのJsonファイルを確認してください。");

		return;
	}

	if (!l_gameObject) 
	{
		FWK_ADD_LOG("ゲームオブジェクトが無効になっており、ゲームオブジェクトのデシリアライズ処理に失敗しました。");

		return;
	}

	// プレハブ名と現在何番目のプレハブかを表す番号付きの名前を格納
	l_gameObject->SetPrefabName              (l_prefabName);
	l_gameObject->SetContainsNumberPrefabName(l_containsNumberPrefabName);

	// プレハブ名からプレハブを取得
	const auto* l_prefab = l_prefabSystem.FindPTRPrefab(l_prefabName);

	if (!l_prefab)
	{
		FWK_ADD_LOG("プレハブが存在しておらず、ゲームオブジェクトのデシリアライズ処理に失敗しました。。");

		return;
	}

	const auto& l_prefabJson = l_prefab->GetREFJson();

	if (l_prefabJson.is_null())
	{
		FWK_ADD_LOG("プレハブのJsonが無効値となっており、ゲームオブジェクトのデシリアライズ処理に失敗しました。。");

		return;
	}

	Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> l_componentLoadList = {};
	std::vector<Struct::ChildDeserializeData>                        l_childLoadList     = {};

	// プレハブの情報を先にデシリアライズ
	DeserializePrefab(l_prefabJson, 
		              l_gameObject,
		              l_childLoadList,
		              l_componentLoadList,
		              a_scene);

	// 各コンポーネントのパラメータを上書きする(座標など)
	DeserializeScene(a_rootJson,   
		             l_childLoadList, 
		             l_componentLoadList, 
		             *l_gameObject,
		             a_scene);

	// デシリアライズした各コンポーネントを、親、子に再帰的に追加
	RecursiveAddChild(l_gameObject, l_childLoadList, a_scene);

	// 親子関係を再帰的に構築
	RecursiveAddChild(l_gameObject, l_childLoadList, a_scene);
}
void FWK::GameObjectJsonConverter::DeserializePrefab(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

		return;
	}
}

void FWK::GameObjectJsonConverter::DeserializePrefab(const nlohmann::json&                                                   a_rootJson, 
	                                                 const std::weak_ptr<GameObject>&                                        a_gameObject,
	                                                       std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                       Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                       Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG("GameObjectが無効のため、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

	const auto& l_key = TransformComponent::GetREFTypeINFO().k_name;

	if (l_key.empty())
	{
		FWK_ADD_LOG("TransformComponentの型情報の文字列が空になっており、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

		return;
	}

	// TransformComponentのプレハブの情報を読み込む
	const auto& l_transformComponentJson = a_rootJson.value(l_key, nlohmann::json{});

	FWK_ASSERT_RETURN_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効になっており、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

	// コンポーネント読み込み用Jonの存在確認、なければreturn
	const auto& l_componentJsonArray = a_rootJson.value(k_componentListJsonKey, nlohmann::json{});

	if (!Utility::IsJsonArray(l_componentJsonArray)) { return; }

	const auto& l_componentFactory = TypeAlias::ComponentSharedFactory::GetInstance();

	// コンポーネントの読み込みと生成
	for (const auto& l_componentJson : l_componentJsonArray)
	{
		if (l_componentJson.is_null()) { continue; }

		// コンポーネントの名前を取得
		const auto& l_typeName = l_componentJson.value(k_componentTypeNameJsonKey, nlohmann::json{});

		// コンポーネントの名前をもとにファクトリーから生成
		// もしインスタンス化されなければcontinue
		const auto& l_component = l_componentFactory.Create(l_typeName);

		if (!l_component) { continue; }

		// コンポーネントにゲームオブジェクト自身のキャッシュを与えデシリアライズ
		l_component->SetOwner             (a_gameObject);
		l_component->DeserializePrefabData(l_componentJson);

		// コンポーネントの実行順序はjsonに保存された順番でありScene情報も
		// 読み込む必要があるため順番を崩さないようにリストで管理
		a_componentSmartPointerVectorArray.Add(l_component);
	}
}
void FWK::GameObjectJsonConverter::DeserializeScene(const nlohmann::json&                                                   a_rootJson, 
	                                                      std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                      Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                                      GameObject&                                                       a_gameObject,
	                                                      Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

		return;
	}

}

nlohmann::json FWK::GameObjectJsonConverter::SerializePrefab(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializeScene(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	return l_rootJson;
}

void FWK::GameObjectJsonConverter::DeserializeComponentObserver(const nlohmann::json& a_rootJson) const
{
	if (a_rootJson.is_null()) { return; }


}

void FWK::GameObjectJsonConverter::RecursiveAddComponent(const std::shared_ptr<GameObject>&                                      a_self,
	                                                           std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                           Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
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

		RecursiveAddComponent(l_childLoad.m_self,
			                  l_childLoad.m_childDeserializeDataList, 
			                  l_childLoad.m_componentSmartPointerVectorArray,
			                  a_scene);
	}
}
void FWK::GameObjectJsonConverter::RecursiveAddChild(const std::shared_ptr<GameObject>& a_parent, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene) const
{
	// 子であろうが一つのリストに格納
	if (!a_parent) { return; }

	// 親子関係を再帰的に構築
	for (auto& l_childLoad : a_childDeserializeDataList)
	{
		if (!l_childLoad.m_self) { continue; }

		a_parent->ApplyParent(l_childLoad.m_self);
		a_scene.AddGameObject(l_childLoad.m_self);
		RecursiveAddChild    (l_childLoad.m_self, l_childLoad.m_childDeserializeDataList, a_scene);
	}
}