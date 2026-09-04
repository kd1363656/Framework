#include "GameObjectPrefabJsonConverter.h"

bool FWK::Converter::GameObjectPrefabJsonConverter::Deserialize(const std::weak_ptr<GameObject>&                                        a_gameObject, 
	                                                            const nlohmann::json&                                                   a_rootJson, 
	                                                                  std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList, 
	                                                                  std::unordered_set<boost::uuids::uuid>&                           a_prefabUUIDSet, 
	                                                                  Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                                  Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null()) { return false; }

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return false; }

	const auto& l_prefabUUID = Utility::DeserializeUUID(a_rootJson, Constant::k_gameObjectPrefabUUIDJsonKey);

	if (l_prefabUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "プレハブのUUIDが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		return false;
	}

	if (!a_prefabUUIDSet.emplace(l_prefabUUID).second)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Constant::k_debugWarningColor, プレハブのUUIDが重複しています、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		return false;
	}

	// Prefabを識別するためのUUIDを格納
	l_gameObject->SetPrefabUUID(l_prefabUUID);

	auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem  ();
	auto* l_prefab       = l_prefabSystem.FindMutablePTRPrefab(l_prefabUUID);

	if (!l_prefab)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "プレハブが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		a_prefabUUIDSet.erase(l_prefabUUID);

		return false;
	}

	// プレハブにゲームオブジェクトが設定されていなければ現在デシリアライズ中のこのゲームオブジェクトをセットしておく
	if (l_prefab->GetREFGameObject().expired())
	{
		l_prefab->SetGameObject(l_gameObject);
	}

	const auto& l_prefabJson = l_prefab->GetREFJson();

	if (l_prefabJson.is_null()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "プレハブが保持しているJsonが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		a_prefabUUIDSet.erase(l_prefabUUID);

		return false; 
	}

	const auto& l_prefabName = l_prefab->GetREFPrefabName();

	// シーンインスタンス名としてプレハブ名を格納しておく
	l_gameObject->SetSceneInstanceName(l_prefabName);

	// コンポーネントのプレハブデータのデシリアライズ
	if (!DeserializePrefabComponent(a_gameObject, l_prefabJson, a_componentSmartPointerVectorArray))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "コンポーネントのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		a_prefabUUIDSet.erase(l_prefabUUID);

		return false;
	}

	if (const auto& l_json = l_prefabJson.value(k_componentEventObserverJsonKey, nlohmann::json{});
		!l_json.is_null() &&
		!DeserializePrefabComponentEventObserver(l_json, *l_gameObject))
	{
		// コンポーネント用オブザーバのデシリアライズ
		// シーン情報は存在しない
		FWK_ADD_LOG(Constant::k_debugWarningColor, "コンポーネントオブザーバーのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		a_prefabUUIDSet.erase(l_prefabUUID);

		return false;
	}

	// プレハブ子ゲームオブジェクトのデシリアライズ
	if (const auto& l_childListJson = l_prefabJson.value(Constant::k_gameObjectChildListJsonKey, nlohmann::json{});
		!l_childListJson.is_null()            &&
		Utility::IsJsonArray(l_childListJson) &&
		!DeserializePrefabChildList(l_childListJson,
		                                a_prefabUUIDSet,
		                                a_childDeserializeDataList,
		                                a_scene))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "子ゲームオブジェクトのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		a_prefabUUIDSet.erase(l_prefabUUID);

		return false;
	}	

	// このPrefab以下のデシリアライズがすべて終了したため、
	// 兄弟Prefabで同じUUIDを使用できるように現在経路から削除する
	a_prefabUUIDSet.erase(l_prefabUUID);

	return true;
}

nlohmann::json FWK::Converter::GameObjectPrefabJsonConverter::Serialize(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_prefabUUID             = a_gameObject.GetREFPrefabUUID            ();
	const auto  l_prefabSceneInstanceNUM = a_gameObject.GetVALPrefabSceneInstanceNUM();

	// Prefab化されていないGameObject、もしくわシーンインスタンス名が
	// 割り当てられていないゲームオブジェクトは、
	// Sceneの保存対象にしない
	if (l_prefabUUID.is_nil() ||
		l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		return {};
	}

	// PrefabUUIDを保存
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_prefabUUID, Constant::k_gameObjectPrefabUUIDJsonKey));

	// コンポーネントのプレハブデータのシリアライズ
	Utility::UpdateJson(l_rootJson, SerializePrefabComponent(a_gameObject));
	
	// コンポーネントオブザーバのシリアライズ
	if (const auto& l_json = SerializePrefabComponentObserver(a_gameObject);
		!l_json.is_null())
	{
		l_rootJson[k_componentEventObserverJsonKey] = l_json;
	}

	// 子ゲームオブジェクトをシリアライズ
	l_rootJson[Constant::k_gameObjectChildListJsonKey] = SerializePrefabChildList(a_gameObject);

	return l_rootJson;
}

bool FWK::Converter::GameObjectPrefabJsonConverter::DeserializePrefabComponent(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return false; }

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。", false);

	// TransformComponentのPrefab情報を取得
	const auto& l_transformComponentJson = a_rootJson.value(Constant::k_gameObjectTransformComponentJsonKey, nlohmann::json{});

	FWK_ASSERT_RETURN_VALUE_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効のため、TransformComponentのプレハブデータのデシリアライズに失敗しました。", false);

	// TransformComponentのプレハブデータを読み込む
	l_transformComponent->DeserializePrefab(l_transformComponentJson);

	// プレハブに保存されているコンポーネントを
	// 保存されている順番のまま生成する
	if (const auto& l_componentJsonArray = a_rootJson.value(Constant::k_gameObjectComponentListJsonKey, nlohmann::json{});
		!l_componentJsonArray.is_null() &&
		Utility::IsJsonArray(l_componentJsonArray))
	{
		const auto& l_componentFactory = TypeAlias::ComponentSharedFactory::GetInstance();

		for (const auto& l_componentJson : l_componentJsonArray)
		{
			// 生成するComponentの型名を取得する
			const auto& l_typeName = l_componentJson.value(Constant::k_gameObjectComponentTypeNameJsonKey, std::string{});

			if (l_typeName.empty())
			{
				FWK_ADD_LOG(Constant::k_debugWarningColor, "ComponentTypeNameが空のため、Componentを生成できませんでした。");

				return false;
			}

			// 型名をもとにComponentを生成する
			const auto& l_component = l_componentFactory.Create(l_typeName);

			if (!l_component) { return false; }

			// ComponentのPrefab情報を読み込む
			l_component->INIT             ();
			l_component->DeserializePrefab(l_componentJson);

			// Scene情報はPrefabと同じ順番で上書きするため
			// Componentの格納順番を維持する
			a_componentSmartPointerVectorArray.Add(l_component);
		}
	}

	return true;
}
bool FWK::Converter::GameObjectPrefabJsonConverter::DeserializePrefabComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
	if (a_rootJson.is_null()) { return false; }

	// ComponentEventObserverのJsonがnull出ない場合std::make_uniqueでインスタンス化
	auto l_componentEventObserver = std::make_shared<Observer<Enum::ComponentEvent>>();

	l_componentEventObserver->INIT();

	l_componentEventObserver->Deserialize(a_rootJson);

	a_gameObject.SetComponentEventObserver(l_componentEventObserver);

	return true;
}
bool FWK::Converter::GameObjectPrefabJsonConverter::DeserializePrefabChildList(const nlohmann::json&                            a_rootJsonArray, 
	                                                                                 std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet, 
	                                                                                 std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
	                                                                                 Scene&                                     a_scene) const
{
	if (a_rootJsonArray.is_null() ||
		!Utility::IsJsonArray(a_rootJsonArray))
	{
		return false; 
	}

	// 子の追加時にvectorが何度も再確保をされることを防ぐ
	a_childDeserializeDataList.reserve(a_childDeserializeDataList.size() + a_rootJsonArray.size());

	for (const auto& l_childJson : a_rootJsonArray)
	{
		// PrefabとSceneでは、子Jsonの配列添え字によって
		// 同じ子GameObjectを対応付けている
		// そのため循環Prefabを除外する場合でもからデータを追加して
		// Prefab側とScene側の添え字がずれないようにする
		a_childDeserializeDataList.emplace_back();

		// 現在追加した子ゲームオブジェクトをリストから取得
		auto& l_childDeserializeData = a_childDeserializeDataList.back();

		if (l_childJson.is_null()) { continue; }

		auto l_child = std::make_shared<GameObject>();

		l_child->INIT();
		
		// GameObjectJsonConverterを直接再帰呼び出しせず、
		// 子GameObject自身のDeserializePrefabを呼び出す
		// 現在のPrefabUUIDSetも引き継ぐため、
		// 孫以下でも同じ親Prefab階層を確認できる
		if (!l_child->DeserializePrefab(l_childJson, 
			                            l_childDeserializeData.m_childDeserializeDataList,
			                            l_childDeserializeData.m_componentSmartPointerVectorArray,
			                            a_prefabUUIDSet,
			                            a_scene))
		{
			// 循環Prefabなどで失敗した場合、
			// m_selfは空のまま残してSceneとの配列添字だけ維持する
			continue;
		}

		// 構造体にSceneデシリアライズ用に自身を格納
		l_childDeserializeData.m_self = l_child;
	}

	return true;
}

nlohmann::json FWK::Converter::GameObjectPrefabJsonConverter::SerializePrefabComponent(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブのシリアライズに失敗しました。", {});

	if (const auto& l_json = l_transformComponent->SerializePrefab();
		!l_json.is_null())
	{
		l_rootJson[Constant::k_gameObjectTransformComponentJsonKey] = l_json;
	}

	// 保存順を保つためにjson::arrayで保存
	      auto  l_componentJsonArray               = nlohmann::json::array                              ();
	const auto& l_componentSmartPointerVectorArray = a_gameObject.GetREFComponentSmartPointerVectorArray();

	for (const auto& l_componentData : l_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		nlohmann::json l_json = {};

		// コンポーネントの名前とそのプレハブ情報を保存
		l_json[Constant::k_gameObjectComponentTypeNameJsonKey] = l_component->GetREFRuntimeTypeINFO().k_name;
		
		Utility::UpdateJson(l_json, l_component->SerializePrefab());

		// もし出力結果がnullならcontinue
		if (l_json.is_null()) { continue; }

		l_componentJsonArray.emplace_back(l_json);
	}

	// 自身を構成するコンポーネントを保存
	l_rootJson[Constant::k_gameObjectComponentListJsonKey] = l_componentJsonArray;

	return l_rootJson;
}
nlohmann::json FWK::Converter::GameObjectPrefabJsonConverter::SerializePrefabComponentObserver(const GameObject& a_gameObject) const
{
	const auto& l_componentEventObserver = a_gameObject.GetVALComponentEventObserver().lock();

	if (!l_componentEventObserver) { return {}; }

	return l_componentEventObserver->Serialize();
}
nlohmann::json FWK::Converter::GameObjectPrefabJsonConverter::SerializePrefabChildList(const GameObject& a_gameObject) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_childSmartPointerVectorArray = a_gameObject.GetREFChildSmartPointerVectorArray();

	// ルートから全ての子情報を再帰的に保存していく
	for (const auto& l_childData : l_childSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		auto l_child = l_childData.m_type.lock();

		if (!l_child ||
			l_child->GetVALIsDestroyed())
		{
			continue; 
		}

		auto l_json = l_child->SerializePrefab();

		if (l_json.is_null()) { continue; }

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}