#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const std::weak_ptr<GameObject>&              a_gameObject, 
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
	DeserializeScene(a_rootJson,
	                 l_childLoadList,
	                 l_componentLoadVectorArray,
	                 *l_gameObject,
		             a_scene);

	// デシリアライズした各コンポーネントを、親、子に再帰的に追加
	RecursiveAddComponent(l_gameObject,
		                  l_componentLoadVectorArray, 
		                  l_childLoadList, 
		                  a_scene);

	// 親子関係を再帰的に構築
	RecursiveAddChild(l_gameObject, 
		              l_childLoadList,
		              a_prefabUUIDSet,
		              a_scene);
}
bool FWK::GameObjectJsonConverter::DeserializePrefab(const std::weak_ptr<GameObject>&                                        a_gameObject, 
	                                                 const nlohmann::json&                                                   a_rootJson,
	                                                       std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                       std::unordered_set<boost::uuids::uuid>&                           a_prefabUUIDSet, 
	                                                       Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, 
	                                                       Scene&                                                            a_scene) const
{
	if (a_rootJson.is_null()) { return false; }

	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return false; }

	const auto& l_prefabUUID = Utility::DeserializeUUID(a_rootJson, k_prefabUUIDJsonKey);

	if (l_prefabUUID.is_nil())
	{
		FWK_ADD_LOG("プレハブのUUIDが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

		return false;
	}

	// Prefabを識別するためのUUIDを格納
	l_gameObject->SetPrefabUUID(l_prefabUUID);

	auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem  ();
	auto* l_prefab       = l_prefabSystem.FindMutablePTRPrefab(l_prefabUUID);

	if (!l_prefab)
	{
		FWK_ADD_LOG("プレハブが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");

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
		FWK_ADD_LOG("プレハブが保持しているJsonが無効となっていおり、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");


		return false; 
	}

	const auto& l_prefabName = l_prefab->GetREFPrefabName();

	// シーンインスタンス名としてプレハブ名を格納しておく
	l_gameObject->SetSceneInstanceName(l_prefabName);

	// コンポーネントのプレハブデータのデシリアライズ
	if (!DeserializePrefabComponent(a_gameObject, l_prefabJson, a_componentSmartPointerVectorArray))
	{
		FWK_ADD_LOG("コンポーネントのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");


		return false;
	}

	// コンポーネント用オブザーバのデシリアライズ
	// シーン情報は存在しない
	if (!DeserializePrefabComponentEventObserver(l_prefabJson, *l_gameObject))
	{
		FWK_ADD_LOG("コンポーネントオブザーバーのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");


		return false;
	}

	// プレハブ子ゲームオブジェクトのデシリアライズ
	if (const auto& l_childListJson = l_prefabJson.value(k_childListJsonKey, nlohmann::json{});
		!l_childListJson.is_null() &&
		Utility::IsJsonArray(l_childListJson))
	{
		if (!DeserializePrefabChildList(l_childListJson,
		                                a_prefabUUIDSet,
		                                a_childDeserializeDataList,
		                                a_scene))
		{
			FWK_ADD_LOG("子ゲームオブジェクトのプレハブデータのデシリアライズに失敗しており、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。");


			return false;
		}
	}	

	return true;
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

	const auto l_prefabSceneInstanceNUM = a_rootJson.value(k_prefabSceneInstanceNUMJsonKey, Constant::k_invalidPrefabSceneInstanceNUM);

	FWK_ASSERT_RETURN_IF(l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM, "PrefabInstanceNUMが無効となっており、GameObjectのScene情報をデシリアライズできませんでした。");

	// シーンに接地されているプレハブで何番目かを格納
	a_gameObject.SetPrefabSceneInstanceNUM(l_prefabSceneInstanceNUM);

	// 保存していたUUIDを取得
	const auto l_sceneInstanceUUID = Utility::DeserializeUUID(a_rootJson, k_sceneInstanceUUIDJsonKey);

	// シーンに配置されているゲームオブジェクトのUUID
	// 他のゲームオブジェクトがこのゲームオブジェクトに瞬時にアクセスしたいときなどに使用する
	a_gameObject.SetSceneInstanceUUID(l_sceneInstanceUUID);

	// PrefabDeserialize時点ではPrefabNameがSceneInstanceNameへ入っている。
	// Scene側に明示的な名前が保存されている場合は、
	// Scene固有情報としてここで上書きする
	if (const auto& l_sceneInstanceName = a_rootJson.value(k_sceneInstanceNameJsonKey, std::string{});
		!l_sceneInstanceName.empty())
	{
		a_gameObject.SetSceneInstanceName(l_sceneInstanceName);
	}

	// コンポーネントのシーン情報のデシリアライズ
	DeserializeSceneComponent(a_rootJson, a_componentSmartPointerVectorArray, a_gameObject);

	// 子ゲームオブジェクトのシーン情報のデシリアライズ
	if (const auto& l_childListJson = a_rootJson.value(k_childListJsonKey, nlohmann::json{});
		!l_childListJson.is_null() &&
		Utility::IsJsonArray(l_childListJson))
	{
		DeserializeSceneChildList(l_childListJson, a_childDeserializeDataList, a_scene);
	}
}

nlohmann::json FWK::GameObjectJsonConverter::SerializePrefab(const GameObject& a_gameObject) const
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
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_prefabUUID, k_prefabUUIDJsonKey));

	// コンポーネントのプレハブデータのシリアライズ
	Utility::UpdateJson(l_rootJson, SerializePrefabComponent(a_gameObject));
	
	// 子ゲームオブジェクトをシリアライズ
	l_rootJson[k_childListJsonKey] = SerializePrefabChildList(a_gameObject);

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializeScene(const GameObject& a_gameObject) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブのシリアライズに失敗しました。", {});

	const auto& l_prefabUUID             = a_gameObject.GetREFPrefabUUID            ();
	const auto& l_sceneInstanceUUID      = a_gameObject.GetREFSceneInstanceUUID     ();
	const auto& l_prefabSceneInstanceNUM = a_gameObject.GetVALPrefabSceneInstanceNUM();
	const auto& l_sceneInstanceName      = a_gameObject.GetREFSceneInstanceName     ();

	if (l_prefabUUID.is_nil() ||
		l_sceneInstanceUUID.is_nil())
	{
		return {};
	}

	// ゲームオブジェクトのシーンインスタンス名と、プレ布武シーンインスタンス番号を保存
	l_rootJson[k_sceneInstanceUUIDJsonKey]      = Utility::SerializeUUID(l_prefabUUID, k_sceneInstanceUUIDJsonKey);
	l_rootJson[k_prefabSceneInstanceNUMJsonKey] = l_prefabSceneInstanceNUM;
	l_rootJson[k_sceneInstanceNameJsonKey]      = l_sceneInstanceName;

	// Prefabを識別するためのUUID
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_prefabUUID, k_prefabUUIDJsonKey));

	// Scene上のGameObject自身を識別するUUID
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_sceneInstanceUUID, k_sceneInstanceUUIDJsonKey));

	// 子ゲームオブジェクトの保存
	l_rootJson[k_childListJsonKey] = SerializeSceneChildList(a_gameObject);

	return l_rootJson;
}

bool FWK::GameObjectJsonConverter::DeserializePrefabComponent(const std::weak_ptr<GameObject>& a_gameObject, const nlohmann::json& a_rootJson, Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return false; }

	const auto& l_transformComponent = l_gameObject->GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのプレハブデータのデシリアライズに失敗しました。", false);

	// TransformComponentのPrefab情報を取得
	const auto& l_transformComponentJson = a_rootJson.value(k_transformComponentJsonKey, nlohmann::json{});

	FWK_ASSERT_RETURN_VALUE_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効のため、TransformComponentのプレハブデータのデシリアライズに失敗しました。", false);

	// TransformComponentのプレハブデータを読み込む
	l_transformComponent->DeserializePrefab(l_transformComponentJson);

	// プレハブに保存されているコンポーネントを
	// 保存されている順番のまま生成する
	if (const auto& l_componentJsonArray = a_rootJson.value(k_componentListJsonKey, nlohmann::json{});
		Utility::IsJsonArray(l_componentJsonArray))
	{
		const auto& l_componentFactory = TypeAlias::ComponentSharedFactory::GetInstance();

		for (const auto& l_componentJson : l_componentJsonArray)
		{
			// 生成するComponentの型名を取得する
			const auto& l_typeName = l_componentJson.value(k_componentTypeNameJsonKey, std::string{});

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

	return true;
}
bool FWK::GameObjectJsonConverter::DeserializePrefabComponentEventObserver(const nlohmann::json& a_rootJson, GameObject& a_gameObject) const
{
	if (a_rootJson.is_null()) { return false; }

	const nlohmann::json l_rootJson = a_rootJson.value(k_componentEventObserverJsonKey, nlohmann::json{});

	if (l_rootJson.is_null()) { return false; }

	auto& l_componentEventObserver = a_gameObject.GetMutableREFComponentEventObserver();

	l_componentEventObserver.Deserialize(l_rootJson);

	return true;
}

bool FWK::GameObjectJsonConverter::DeserializePrefabChildList(const nlohmann::json&                            a_rootJsonArray, 
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
		l_child->DeserializePrefab(l_childJson, 
			                       l_childDeserializeData.m_childDeserializeDataList,
			                       l_childDeserializeData.m_componentSmartPointerVectorArray,
			                       a_prefabUUIDSet,
			                       a_scene);

		// 構造体にSceneデシリアライズ用に自身を格納
		l_childDeserializeData.m_self = l_child;
	}

	return true;
}

void FWK::GameObjectJsonConverter::DeserializeSceneComponent(const nlohmann::json& a_rootJson, Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, GameObject& a_gameObject) const
{
	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_IF(!l_transformComponent, "TransformComponentが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

	// TransformComponentのプレハブの情報を読み込む
	const auto& l_transformComponentJson = a_rootJson.value(k_transformComponentJsonKey, nlohmann::json{});

	FWK_ASSERT_RETURN_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効になっており、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

	// TransformComponentのシーンデータをデシリアライズ
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
}

void FWK::GameObjectJsonConverter::DeserializeSceneChildList(const nlohmann::json& a_rootJsonArray, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene) const
{
	// リストが空なら読み込めていないのでreturn
	if (a_childDeserializeDataList.empty() ||
		a_rootJsonArray.is_null()          ||
		!Utility::IsJsonArray(a_rootJsonArray))
	{
		return; 
	}

	// Json配列のサイズが一致しなければreturn
	if (a_childDeserializeDataList.size() != a_rootJsonArray.size())
	{
		FWK_ADD_LOG("子の数がPrefabとSceneで一致しないためSceneの子情報のデシリアライズに失敗しました。");

		return;
	}

	// 子の保存順序はJson::arrayで保証されているので安全にScene情報を読み取ることができる
	for (std::size_t l_i = 0ULL; l_i < a_rootJsonArray.size(); ++l_i)
	{
		const auto& l_json = a_rootJsonArray[l_i];

		if (l_json.is_null()) { continue; }

		auto& l_childDeserializeData = a_childDeserializeDataList[l_i];

		if (!l_childDeserializeData.m_self) { continue; }

		l_childDeserializeData.m_self->DeserializeScene(l_json, 
			                                            l_childDeserializeData.m_childDeserializeDataList, 
			                                            l_childDeserializeData.m_componentSmartPointerVectorArray, 
			                                            a_scene);
	}
}

nlohmann::json FWK::GameObjectJsonConverter::SerializePrefabComponent(const GameObject& a_gameObject) const
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
	      auto  l_componentJsonArray               = nlohmann::json::array                              ();
	const auto& l_componentSmartPointerVectorArray = a_gameObject.GetREFComponentSmartPointerVectorArray();

	for (const auto& l_componentData : l_componentSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_component = l_componentData.m_type;

		if (!l_component) { continue; }

		nlohmann::json l_json = {};

		// コンポーネントの名前とそのプレハブ情報を保存
		l_json[k_componentTypeNameJsonKey] = l_component->GetREFRuntimeTypeINFO().k_name;
		
		Utility::UpdateJson(l_json, l_component->SerializePrefab());

		// もし出力結果がnullならcontinue
		if (l_json.is_null()) { continue; }

		l_componentJsonArray.emplace_back(l_json);
	}

	// 自身を構成するコンポーネントを保存
	l_rootJson[k_componentListJsonKey] = l_componentJsonArray;

	return l_rootJson;
}

nlohmann::json FWK::GameObjectJsonConverter::SerializePrefabComponentObserver(const GameObject& a_gameObject) const
{
	const auto& l_componentEventObserver = a_gameObject.GetREFComponentEventObserver();

	nlohmann::json l_rootJson = {};

	l_rootJson[k_componentEventObserverJsonKey] = l_componentEventObserver.Serialize();

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializePrefabChildList(const GameObject& a_gameObject) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	// ルートから全ての子情報を再帰的に保存していく
	for (const auto& l_childData : a_gameObject.GetREFChildSmartPointerVectorArray().GetREFArrayElementDataList())
	{
		auto l_child = l_childData.m_type.lock();

		if (!l_child) { continue; }

		auto l_json = l_child->SerializePrefab();

		if (l_json.is_null()) { continue; }

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}

nlohmann::json FWK::GameObjectJsonConverter::SerializeSceneComponent(const GameObject& a_gameObject) const
{
	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	if(!l_transformComponent)
	{
		FWK_ADD_LOG("シーンデータのシリアライズ時にTransformComponentが無効だったため、シリアライズ処理に失敗しました。");

		return {};
	}

	nlohmann::json l_rootJson = {};

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
		l_json[k_componentTypeNameJsonKey] = l_component->GetREFRuntimeTypeINFO().k_name;
		
		Utility::UpdateJson(l_json, l_component->SerializeScene());

		// もし出力結果がnullならcontinue
		if (l_json.is_null()) { continue; }

		l_componentJsonArray.emplace_back(l_json);
	}

	// 自身を構成するコンポーネントを保存
	l_rootJson[k_componentListJsonKey] = l_componentJsonArray;

	return l_rootJson;
}
nlohmann::json FWK::GameObjectJsonConverter::SerializeSceneChildList(const GameObject& a_gameObject) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	// ルートから全ての子情報を再帰的に保存していく
	for (const auto& l_childData : a_gameObject.GetREFChildSmartPointerVectorArray().GetREFArrayElementDataList())
	{
		auto l_child = l_childData.m_type.lock();

		if (!l_child) { continue; }

		auto l_json = l_child->SerializeScene();

		if (l_json.is_null()) { continue; }

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
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

		// 子のコンポーネントも再帰的に追加していく
		RecursiveAddComponent(l_childLoad.m_self,
			                  l_childLoad.m_componentSmartPointerVectorArray,
			                  l_childLoad.m_childDeserializeDataList, 
			                  a_scene);
	}
}
void FWK::GameObjectJsonConverter::RecursiveAddChild(const std::shared_ptr<GameObject>&               a_parent, 
	                                                       std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, 
	                                                       std::unordered_set<boost::uuids::uuid>&    a_prefabUUIDSet, 
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
		if (!a_parent->ApplyParent(l_childLoad.m_self, a_prefabUUIDSet)) { continue; }

		// 親子関係を構築できたGameObjectだけをSceneへ追加する
		a_scene.AddGameObject(l_childLoad.m_self);

		// 子のPrefabUUIDがSetへ入った状態で
		// 孫以下の親子関係を構築する
		RecursiveAddChild(l_childLoad.m_self, 
			             l_childLoad.m_childDeserializeDataList,
			             a_prefabUUIDSet,
			             a_scene);
	}
}