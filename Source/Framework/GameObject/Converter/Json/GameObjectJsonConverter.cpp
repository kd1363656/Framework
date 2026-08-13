#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const std::weak_ptr<GameObject>&              a_gameObject,
	                                           const nlohmann::json&                         a_rootJson, 
	                                                 std::unordered_set<boost::uuids::uuid>& a_parentPrefabUUIDSet,
	                                                 Scene&                                  a_scene) const
{
	if (a_rootJson.is_null()) 
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのデシリアライズに失敗しました。");

		return; 
	}

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) 
	{
		FWK_ADD_LOG("ゲームオブジェクトが無効になっており、ゲームオブジェクトのデシリアライズ処理に失敗しました。");

		return;
	}

	// ゲームオブジェクトのシーンインスタンス名を読み取る
	// インスタンス名が空でない場合にのみ格納
	if (const auto& l_sceneInstanceName = a_rootJson.value(k_sceneInstanceNameJsonKey, std::string{});
		!l_sceneInstanceName.empty())
	{
		l_gameObject->SetSceneInstanceName(l_sceneInstanceName);
	}

	Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> l_componentLoadVectorArray = {};
	std::vector<Struct::ChildDeserializeData>                        l_childLoadList            = {};

	// プレハブの情報を先にデシリアライズ
	DeserializePrefab(l_prefabJson, 
		              l_gameObject,
		              l_childLoadList,
		              l_componentLoadVectorArray,
		              a_parentPrefabUUIDSet,
		              a_scene);

	// 各コンポーネントのパラメータを上書きする(座標など)
	DeserializeScene(a_rootJson,   
		             *l_gameObject,
		             l_childLoadList, 
		             l_componentLoadVectorArray, 
		             a_scene);

	// デシリアライズした各コンポーネントを、親、子に再帰的に追加
	RecursiveAddComponent(l_gameObject,
		                  l_componentLoadVectorArray, 
		                  l_childLoadList, 
		                  a_scene);

	// 親子関係を再帰的に構築
	RecursiveAddChild(l_gameObject, 
		              l_childLoadList,
		              a_parentPrefabUUIDSet,
		              a_scene);

	// このルート以下の読み込みが完了したため登録を解除する
	a_parentPrefabUUIDSet.erase(l_prefabUUID);
}
void FWK::GameObjectJsonConverter::DeserializePrefab(const std::weak_ptr<GameObject>&              a_gameObject, 
	                                                 const nlohmann::json&                         a_rootJson, 
	                                                       std::unordered_set<boost::uuids::uuid>& a_parentPrefabUUIDSet, 
	                                                       Scene&                                  a_scene) const
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

	// Prefabの検索や同一性判定ではSceneInstanceNameを使用せず、
	// 永続的な識別子であるPrefabUUIDを使用する
	const auto& l_prefabUUID = Utility::DeserializeUUID(a_rootJson, k_childListJsonKey);

	if (l_prefabUUID.is_nil())
	{
		FWK_ADD_LOG("PrefabUUIDが無効のため、GameObjectをデシリアライズできませんでした。");

		return;
	}

	// プレハブ識別UUIDを格納
	l_gameObject->SetPrefabUUID(l_prefabUUID);
	
	// プレハブ名からプレハブを取得
	const auto& l_prefabSystem = a_scene.GetREFPrefabSystem  ();
	const auto* l_prefab       = l_prefabSystem.FindPTRPrefab(l_prefabUUID);

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

	// ルートPrefabUUIDを、現在のPrefab階層へ登録する
	if (!a_parentPrefabUUIDSet.emplace(l_prefabUUID).second)
	{
		FWK_ADD_LOG("ルートGameObjectと同じPrefabNameが親Prefab階層にすでに存在しています。");

		return;
	}

	// ゲームオブジェクトのシーンインスタンス名を読み取る
	const auto& l_sceneInstanceName = a_rootJson.value(k_sceneInstanceNameJsonKey, std::string{});

	if (l_sceneInstanceName.empty())
	{
		FWK_ADD_LOG("名前が空のシーンインスタンスががJsonファイルに含まれています。SceneのJsonファイルを確認してください。");

		return;
	}

	l_gameObject->SetSceneInstanceName(l_sceneInstanceName);

	const auto& l_prefabUUIDString = a_rootJson.value     (k_prefabUUIDJsonKey, std::string{});
	const auto& l_prefabUUID       = Utility::StringToUUID(l_prefabUUIDString);

	if (l_prefabUUID.is_nil())
	{
		FWK_ADD_LOG("PrefabUUIDが無効のため、GameObjectをデシリアライズできませんでした。");

		return;
	}

	a_parentPrefabUUIDSet.emplace(l_prefabUUID);

	Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>> l_componentLoadVectorArray = {};
	std::vector<Struct::ChildDeserializeData>                        l_childLoadList            = {};

	// プレハブの情報を元にデシリアライズ
	DeserializePrefab(a_rootJson,
		              l_gameObject,
		              l_childLoadList,
		              l_componentLoadVectorArray,
		              l_parentPrefabUUIDSet,
		              a_scene);

	// デシリアライズした各コンポーネントを、親、子に再帰的に追加
	RecursiveAddComponent(l_gameObject,
		                  l_componentLoadVectorArray,
		                  l_childLoadList, 
		                  a_scene);

	// 親子関係を再帰的に構築
	RecursiveAddChild(l_gameObject, 
		              l_childLoadList,
		              l_parentPrefabUUIDSet,
		              a_scene);
}

void FWK::GameObjectJsonConverter::DeserializePrefab(const nlohmann::json&                                                   a_rootJson, 
	                                                 const std::weak_ptr<GameObject>&                                        a_gameObject, 
	                                                       std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                       Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                       std::unordered_set<boost::uuids::uuid>&                           a_parentPrefabUUIDSet, 
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
		FWK_ADD_LOG("GameObjectが無効のため、GameObjectのPrefabのデシリアライズに失敗しました。");

		return;
	}

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効のため、GameObjectのPrefabのデシリアライズに失敗しました。");

	// TransformComponentのPrefab情報を取得
	const auto& l_transformComponentJson = a_rootJson.value(k_transformComponentJsonKey, nlohmann::json{});

	FWK_ASSERT_RETURN_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効のため、TransformComponentのPrefab情報をデシリアライズできませんでした。");

	// TransformComponentのPrefab情報を読み込む
	l_transformComponent->DeserializePrefab(l_transformComponentJson);

	// Prefabに保存されているコンポーネントを
	// 保存されている順番のまま生成する
	if (const auto& l_componentJsonArray = a_rootJson.value(k_componentListJsonKey, nlohmann::json{});
		Utility::IsJsonArray(l_componentJsonArray))
	{
		const auto& l_componentFactory = TypeAlias::ComponentSharedFactory::GetInstance();

		for (const auto& l_componentJson : l_componentJsonArray)
		{
			// 生成するComponentの型名を取得する
			const auto& l_typeName = l_componentJson.value(k_typeNameJsonKey, std::string{});

			if (l_typeName.empty())
			{
				FWK_ADD_LOG("ComponentTypeNameが空のため、Componentを生成できませんでした。");

				continue;
			}

			// 型名をもとにComponentを生成する
			const auto& l_component = l_componentFactory.Create(l_typeName);

			if (!l_component) { continue; }

			// ComponentのPrefab情報を読み込む
			l_component->INIT             ();
			l_component->DeserializePrefab(l_componentJson);

			// Scene情報はPrefabと同じ順番で上書きするため
			// Componentの格納順番を維持する
			a_componentSmartPointerVectorArray.Add(l_component);
		}
	}

	// ComponentEventObserverのPrefab情報を読み込む
	DeserializeComponentEventObserver(a_rootJson, *l_gameObject);

	// 現在の親Prefab名一覧を引き継ぎながら、
	// 子GameObjectを読み込む
	DeserializeChildPrefab(a_rootJson,
		                   a_childDeserializeDataList,
		                   a_parentPrefabUUIDSet,
		                   a_scene);
}
void FWK::GameObjectJsonConverter::DeserializeScene(const nlohmann::json&                                                   a_rootJson,
	                                                      GameObject&                                                       a_gameObject,
	                                                      std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                      Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                                      Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

		return;
	}

	const auto l_prefabSceneInstanceNUM = a_rootJson.value(k_prefabSceneInstanceNUMJsonKey, Constant::k_invalidPrefabInstanceNUM);

	FWK_ASSERT_RETURN_IF(l_prefabSceneInstanceNUM == Constant::k_invalidPrefabInstanceNUM, "PrefabInstanceNUMが無効となっており、GameObjectのScene情報をデシリアライズできませんでした。");

	a_gameObject.SetPrefabSceneInstanceNUM(l_prefabSceneInstanceNUM);

	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

	// 保存していたUUIDを取得
	const auto l_sceneInstanceUUID = Utility::DeserializeUUID(a_rootJson, k_sceneInstanceUUIDJsonKey);

	a_gameObject.SetSceneInstanceUUID(l_sceneInstanceUUID);
	
	// TransformComponentのプレハブの情報を読み込む
	const auto& l_transformComponentJson = a_rootJson.value(k_transformComponentJsonKey, nlohmann::json{});

	FWK_ASSERT_RETURN_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効になっており、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

	l_transformComponent->DeserializeScene(l_transformComponentJson);

	// コンポーネント読み込み用Jonの存在確認、なければreturn
	if (const auto& l_componentJsonArray = a_rootJson.value(k_componentListJsonKey, nlohmann::json{});
		Utility::IsJsonArray(l_componentJsonArray))
	{
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

			l_component->DeserializeScene(l_json);
		}
	}

	DeserializeChildScene(a_rootJson, a_childDeserializeDataList, a_scene);
}

nlohmann::json FWK::GameObjectJsonConverter::SerializePrefab(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブのシリアライズに失敗しました。", {});

	// PrefabUUIDを文字列にして保存
	l_rootJson[k_prefabUUIDJsonKey] = boost::uuids::to_string(a_gameObject.GetREFPrefabUUID());
	
	if (const auto& l_json = l_transformComponent->SerializePrefab();
		!l_json.is_null())
	{
		l_rootJson[k_transformComponentJsonKey] = l_json;
	}

	// 保存順を保つためにjson::arrayで保存
	auto l_componentJsonArray = nlohmann::json::array();

	for (const auto& l_componentData : a_gameObject.GetREFComponentSmartPointerVectorArray().GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		nlohmann::json l_json = {};

		// コンポーネントの名前とそのプレハブ情報を保存
		l_json[k_typeNameJsonKey] = l_component->GetREFRuntimeTypeINFO().k_name;
		
		Utility::UpdateJson(l_json, l_component->SerializePrefab());

		// もし出力結果がnullならcontinue
		if (l_json.is_null()) { continue; }

		l_componentJsonArray.emplace_back(l_json);
	}

	// 自身を構成するコンポーネントを保存
	l_rootJson[k_componentListJsonKey] = l_componentJsonArray;

	// コンポーネントオブザーバーをデシリアライズ
	Utility::UpdateJson(l_rootJson, SerializeComponentObserver(a_gameObject));

	// 子ゲームオブジェクトの保存
	Utility::UpdateJson(l_rootJson, SerializeChildPrefab(a_gameObject));

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializeScene(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_prefabUUID             = a_gameObject.GetREFPrefabUUID            ();
	const auto  l_prefabSceneInstanceNUM = a_gameObject.GetVALPrefabSceneInstanceNUM();

	// Prefab化されていないGameObject、もしくわシーンインスタンス名が
	// 割り当てられていないゲームオブジェクトは、
	// Sceneの保存対象にしない
	if (l_prefabUUID.is_nil() ||
		l_prefabSceneInstanceNUM == Constant::k_invalidPrefabInstanceNUM)
	{
		return {};
	}

	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブのシリアライズに失敗しました。", {});

	// ゲームオブジェクトのシーンインスタンス名と、プレ布武シーンインスタンス番号を保存
	l_rootJson[k_sceneInstanceNameJsonKey]      = a_gameObject.GetREFSceneInstanceName     ();
	l_rootJson[k_prefabSceneInstanceNUMJsonKey] = a_gameObject.GetVALPrefabSceneInstanceNUM();

	// Prefabを識別するためのUUID
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFPrefabUUID(), k_prefabUUIDJsonKey));

	// Scene上のGameObject自身を識別するUUID
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(a_gameObject.GetREFSceneInstanceUUID(), k_sceneInstanceUUIDJsonKey));

	// 同じPrefabから生成された何番目のInstanceなのかを保存
	l_rootJson[k_prefabSceneInstanceNUMJsonKey] = l_prefabSceneInstanceNUM;

	if (const auto& l_json = l_transformComponent->SerializeScene();
		!l_json.is_null())
	{
		l_rootJson[k_transformComponentJsonKey] = l_json;
	}

	// 保存順を保つためにjson::arrayで保存
	auto l_componentJsonArray = nlohmann::json::array();

	const auto& l_componentSmartPointerVectorArray = a_gameObject.GetREFComponentSmartPointerVectorArray();

	for (const auto& l_componentData : l_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		nlohmann::json l_json = {};

		// コンポーネントの名前とそのプレハブ情報を保存
		l_json[k_typeNameJsonKey] = l_component->GetREFRuntimeTypeINFO().k_name;
		
		Utility::UpdateJson(l_json, l_component->SerializeScene());

		// もし出力結果がnullならcontinue
		if (l_json.is_null()) { continue; }

		l_componentJsonArray.emplace_back(l_json);
	}

	// 自身を構成するコンポーネントを保存
	l_rootJson[k_componentListJsonKey] = l_componentJsonArray;

	// 子ゲームオブジェクトの保存
	Utility::UpdateJson(l_rootJson, SerializeChildScene(a_gameObject));

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
	                                                            std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
	                                                            std::unordered_set<boost::uuids::uuid>&   a_parentPrefabNameSet, 
	                                                            Scene&                                     a_scene) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_childJsonArray = a_rootJson.value(k_childListJsonKey, nlohmann::json::array());

	if (!Utility::IsJsonArray(l_childJsonArray)) { return; }

	// 子の追加時にvectorが何度も再確保をされることを防ぐ
	a_childDeserializeDataList.reserve(a_childDeserializeDataList.size() + l_childJsonArray.size());

	for (const auto& l_childJson : l_childJsonArray)
	{
		// PrefabとSceneでは、子Jsonの配列添え字によって
		// 同じ子GameObjectを対応付けている
		// そのため循環Prefabを除外する場合でもからデータを追加して
		// Prefab側とScene側の添え字がずれないようにする
		a_childDeserializeDataList.emplace_back();

		// 現在追加した子ゲームオブジェクトをリストから取得
		auto& l_childDeserializeData = a_childDeserializeDataList.back();

		if (l_childJson.is_null()) { continue; }

		// 子のプレハブ名を読み込む
		const auto& l_prefabUUID = Utility::DeserializeUUID(l_childJson, k_prefabUUIDJsonKey);
		
		if (l_prefabUUID.is_nil())
		{
			FWK_ADD_LOG("子GameObjectのPrefabUUIDが無効となっており、子GameObjectをデシリアライズできませんでした。");

			continue;
		}

		// emplaceの戻り値を使うことで
		// contains()とemplace()による二重検索を避ける
		// 登録できなかった場合は、現在の親階層に
		// 同じPrefab名が既に存在している
		if (!a_parentPrefabNameSet.emplace(l_prefabUUID).second)
		{
			FWK_ADD_LOG("親階層と同じPrefabUUIDを持つ子GameObjectを検出したため、子GameObjectのデシリアライズを除外しました。");

			continue;
		}

		auto l_child = std::make_shared<GameObject>();

		l_child->INIT         ();
		l_child->SetPrefabUUID(l_prefabUUID);
		
		// GameObjectJsonConverterを直接再帰呼び出しせず、
		// 子GameObject自身のDeserializePrefabを呼び出す
		// 現在のPrefabUUIDSetも引き継ぐため、
		// 孫以下でも同じ親Prefab階層を確認できる
		l_child->DeserializePrefab(l_childJson, 
			                       l_childDeserializeData.m_childDeserializeDataList,
			                       l_childDeserializeData.m_componentSmartPointerVectorArray,
			                       a_parentPrefabNameSet,
			                       a_scene);

		// この子以下のデシリアライズが終了したため、
		// 同名Prefabを持つ兄弟が読み込目るように登録を解除する
		a_parentPrefabNameSet.erase(l_prefabUUID);

		// 構造体にSceneデシリアライズ用に自身を格納
		l_childDeserializeData.m_self = l_child;
	}
}

void FWK::GameObjectJsonConverter::DeserializeChildScene(const nlohmann::json& a_rootJson, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene) const
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

nlohmann::json FWK::GameObjectJsonConverter::SerializeComponentObserver(const GameObject& a_gameObject) const
{
	const auto& l_componentEventObserver = a_gameObject.GetREFComponentEventObserver();

	nlohmann::json l_rootJson = {};

	l_rootJson[k_componentEventObserverJsonKey] = l_componentEventObserver.Serialize();

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializeChildPrefab(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson       = {};
	auto           l_childJsonArray = nlohmann::json::array();

	// ルートから全ての子情報を再帰的に保存していく
	for (const auto& l_childData : a_gameObject.GetREFChildSmartPointerVectorArray().GetREFArrayElementDataList())
	{
		auto l_child = l_childData.m_type.lock();

		if (!l_child) { continue; }

		auto l_json = l_child->SerializePrefab();

		if (l_json.is_null()) { continue; }

		l_childJsonArray.emplace_back(l_json);
	}

	l_rootJson[k_childListJsonKey] = l_childJsonArray;

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializeChildScene(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson       = {};
	auto           l_childJsonArray = nlohmann::json::array();

	// ルートから全ての子情報を再帰的に保存していく
	for (const auto& l_childData : a_gameObject.GetREFChildSmartPointerVectorArray().GetREFArrayElementDataList())
	{
		auto l_child = l_childData.m_type.lock();

		if (!l_child) { continue; }

		auto l_json = l_child->SerializeScene();

		if (l_json.is_null()) { continue; }

		l_childJsonArray.emplace_back(l_json);
	}

	l_rootJson[k_childListJsonKey] = l_childJsonArray;

	return l_rootJson;
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
void FWK::GameObjectJsonConverter::RecursiveAddChild(const std::shared_ptr<GameObject>&               a_parent, 
	                                                       std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
	                                                       std::unordered_set<boost::uuids::uuid>&    a_parentPrefabUUIDSet,
	                                                       Scene&                                     a_scene) const
{
	// 子であろうが一つのリストに格納
	if (!a_parent) { return; }

	// 親子関係を再帰的に構築
	for (auto& l_childLoad : a_childDeserializeDataList)
	{
		if (!l_childLoad.m_self) { continue; }

		const auto& l_childPrefabUUID = l_childLoad.m_self->GetREFPrefabUUID();

		// 同じPrefab名が親経路に存在する場合や、
		// GameObjectの親子関係を構築できなかった場合は追加しない
		if (!a_parent->ApplyParent(l_childLoad.m_self, a_parentPrefabUUIDSet)) { continue; }

		// 親子関係を構築できたGameObjectだけをSceneへ追加する
		a_scene.AddGameObject(l_childLoad.m_self);

		// 子のPrefabUUIDがSetへ入った状態で
		// 孫以下の親子関係を構築する
		RecursiveAddChild(l_childLoad.m_self, 
			             l_childLoad.m_childDeserializeDataList,
			             a_parentPrefabUUIDSet,
			             a_scene);

		// この子以下の処理が完了したため
		// 同じPrefabを持つ筐体を追加できるように登録解除する
		a_parentPrefabUUIDSet.erase(l_childPrefabUUID);
	}
}