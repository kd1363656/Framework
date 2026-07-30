#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Scene& a_scene) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのデシリアライズに失敗しました。");
		return; 
	}

	const auto& l_prefabSystem = a_scene.GetREFPrefabSystem();
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
		             *l_gameObject,
		             l_childLoadList, 
		             l_componentLoadList, 
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

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject)
	{
		FWK_ADD_LOG("ゲームオブジェクトが無効になっており、ゲームオブジェクトのデシリアライズ処理に失敗しました。");

		return;
	}

	Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> l_componentLoadList = {};
	std::vector<Struct::ChildDeserializeData>                        l_childLoadList     = {};

	// プレハブの情報を元にデシリアライズ
	DeserializePrefab(a_rootJson,
		              l_gameObject,
		              l_childLoadList,
		              l_componentLoadList,
		              a_scene);

	// デシリアライズした各コンポーネントを、親、子に再帰的に追加
	RecursiveAddChild(l_gameObject, l_childLoadList, a_scene);

	// 親子関係を再帰的に構築
	RecursiveAddChild(l_gameObject, l_childLoadList, a_scene);
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

	const auto& l_transformComponentKey = TransformComponent::GetREFTypeINFO().k_name;

	if (l_transformComponentKey.empty())
	{
		FWK_ADD_LOG("TransformComponentの型情報の文字列が空になっており、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

		return;
	}

	// TransformComponentのプレハブの情報を読み込む
	const auto& l_transformComponentJson = a_rootJson.value(l_transformComponentKey, nlohmann::json{});

	FWK_ASSERT_RETURN_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効になっており、ゲームオブジェクトのプレハブのデシリアライズに失敗しました。");

	l_transformComponent->DeserializePrefabData(a_rootJson);

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

	// コンポーネントオブザーバーのデシリアライズ
	DeserializeComponentEventObserver(a_rootJson, *l_gameObject);

	// 子ゲームオブジェクトのデシリアライズ
	DeserializeChildPrefab(a_rootJson,
		                   a_gameObject,
		                   a_childDeserializeDataList,
		                   a_scene);
}
void FWK::GameObjectJsonConverter::DeserializeScene(const nlohmann::json&                                                   a_rootJson, 
	                                                const GameObject&                                                       a_gameObject,
	                                                      std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                      Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                                      Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

	const auto& l_transformComponentKey = TransformComponent::GetREFTypeINFO().k_name;

	if (l_transformComponentKey.empty())
	{
		FWK_ADD_LOG("TransformComponentの型情報の文字列が空になっており、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

		return;
	}

	// TransformComponentのプレハブの情報を読み込む
	const auto& l_transformComponentJson = a_rootJson.value(l_transformComponentKey, nlohmann::json{});

	FWK_ASSERT_RETURN_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効になっており、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

	// コンポーネント読み込み用Jonの存在確認、なければreturn
	const auto& l_componentJsonArray = a_rootJson.value(k_componentListJsonKey, nlohmann::json{});

	if (!Utility::IsJsonArray(l_componentJsonArray)) { return; }

	auto& l_componentList = a_componentSmartPointerVectorArray.GetMutableREFArrayElementDataList();;

	// コンポーネント数が一致しない場合コンポーネントのデシリアライズを行わない
	if (l_componentJsonArray.size() != l_componentList.size())
	{
		FWK_ADD_LOG("コンポーネントの数がPrefabとSceneで一致しないためComponentのScene情報のデシリアライズに失敗しました。");
	
		return;
	}

	for (std::size_t l_i = 0U; l_i < l_componentJsonArray.size(); ++l_i)
	{
		const auto& l_json = l_componentJsonArray[l_i];

		if (l_json.is_null()) { continue; }

		// 各コンポーネントのScene情報を読み込むJsonArrayで保存する関係上
		// 絶対に前回と同じコンポーネントの格納順番なので安全にインデックスを指定して
		// 読み込むことが可能
		const auto& l_component = l_componentList[l_i].m_type;

		if (!l_component) { continue; }

		l_component->DeserializeSpawnData(l_json);
	}

	DeserializeChildScene(a_rootJson, 
		                  a_gameObject,
		                  a_childDeserializeDataList,
		                  a_scene);
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

void FWK::GameObjectJsonConverter::DeserializeComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
	if (a_rootJson.is_null()) { return; }

	const nlohmann::json l_rootJson = a_rootJson.value(k_componentEventObserverJsonKey, nlohmann::json{});

	if (l_rootJson.is_null()) { return; }

	auto& l_componentEventObserver = a_gameObject.GetMutableREFComponentEventObserver();

	l_componentEventObserver.Deserialize(l_rootJson);
}

void FWK::GameObjectJsonConverter::DeserializeChildPrefab(const nlohmann::json&                            a_rootJson, 
	                                                      const std::weak_ptr<GameObject>&                 a_gameObject, 
	                                                            std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
	                                                            Scene&                                     a_scene) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_childJsonArray = a_rootJson.value(k_childListJsonKey, nlohmann::json::array());

	if (!Utility::IsJsonArray(l_childJsonArray)) { return; }

	for (const auto& l_childJson : l_childJsonArray)
	{
		if (l_childJson.is_null()) { continue; }

		// リストが回るたびに子ゲームオブジェクトが"Json"保存前に存在していたということなので
		// 毎回ゲームオブジェクトを生成
		Struct::ChildDeserializeData l_childDeserializeData = {};

		// 子を初期化
		auto l_child = std::make_shared<GameObject>();

		// 子のプレハブ名を読み込む
		const auto& l_prefabName               = l_childJson.value(k_prefabNameJsonKey,               std::string{});
		const auto& l_containsNumberPrefabName = l_childJson.value(k_contanisNumberPrefabNameJsonKey, std::string{});

		// プレハブ名と何番目のプレハブかを含む名前を格納
		l_child->SetPrefabName              (l_prefabName);
		l_child->SetContainsNumberPrefabName(l_containsNumberPrefabName);

		l_child->DeserializePrefab(l_childJson, 
			                       l_childDeserializeData.m_childDeserializeDataList,
			                       l_childDeserializeData.m_componentSmartPointerVectorArray,
			                       a_scene);

		// 構造体にSceneデシリアライズ用に自身を格納
		l_childDeserializeData.m_self = l_child;

		// ロード用リストに格納
		a_childDeserializeDataList.emplace_back(l_childDeserializeData);
	}
}

void FWK::GameObjectJsonConverter::DeserializeChildScene(const nlohmann::json&                            a_rootJson, 
	                                                     const GameObject&                                a_gameObject, 
	                                                           std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList,
	                                                           Scene&                                     a_scene) const
{
	// リストが空なら読み込めていないのでreturn
	if (a_childDeserializeDataList.empty() ||
		a_rootJson.is_null())
	{
		return; 
	}
	
	const auto& l_childJsonArray = a_rootJson.value(k_childListJsonKey, nlohmann::json::array());

	// Jsonが配列でない、もしくはサイズが一致しなければreturn
	if (!Utility::IsJsonArray(l_childJsonArray)) { return; }

	if (a_childDeserializeDataList.size() != l_childJsonArray.size())
	{
		FWK_ADD_LOG("子の数がPrefabとSceneで一致しないためSceneの子情報のデシリアライズに失敗しました。");

		return;
	}

	// 子の保存順序はJson::arrayで保証されているので安全にScene情報を読み取ることができる
	for (std::size_t l_i = 0ULL; l_i < l_childJsonArray.size(); ++l_i)
	{
		const auto& l_json = l_childJsonArray[l_i];

		if (l_json.is_null()) { continue; }

		auto& l_childDeserializeData = a_childDeserializeDataList[l_i];

		if (!l_childDeserializeData.m_self) { continue; }

		l_childDeserializeData.m_self->DeserializeScene(l_json, 
			                                            l_childDeserializeData.m_childDeserializeDataList, 
			                                            l_childDeserializeData.m_componentSmartPointerVectorArray, 
			                                            a_scene);
	}
}

void FWK::GameObjectJsonConverter::RecursiveAddComponent(const std::shared_ptr<GameObject>&                                      a_self,
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

		RecursiveAddComponent(l_childLoad.m_self,
			                  l_childLoad.m_componentSmartPointerVectorArray,
			                  l_childLoad.m_childDeserializeDataList, 
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