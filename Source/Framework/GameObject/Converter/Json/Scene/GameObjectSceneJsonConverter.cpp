#include "GameObjectSceneJsonConverter.h"

bool FWK::Converter::GameObjectSceneJsonConverter::Deserialize(const nlohmann::json&                                                   a_rootJson, 
	                                                                 std::vector<Struct::ChildDeserializeData>&                        a_childDeserializeDataList,
	                                                                 Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray,
	                                                                 GameObject&                                                       a_gameObject, 
	                                                                 Scene&                                                            a_scene) const
{
    if (a_rootJson.is_null())
	{
		FWK_ADD_LOG("RootJsonが無効のため、ゲームオブジェクトのシーンのデシリアライズに失敗しました。");

		return false;
	}

	const auto l_prefabSceneInstanceNUM = a_rootJson.value(k_prefabSceneInstanceNUMJsonKey, Constant::k_invalidPrefabSceneInstanceNUM);

	if (l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM)
	{
		FWK_ADD_LOG("PrefabSceneInstanceNUMが無効のため、GameObjectのSceneデータをデシリアライズできませんでした。");

		return false;
	}
	
	// 保存していたUUIDを取得
	const auto l_sceneInstanceUUID = Utility::DeserializeUUID(a_rootJson, k_sceneInstanceUUIDJsonKey);

	if (l_sceneInstanceUUID.is_nil())
	{
		FWK_ADD_LOG("SceneInstanceUUIDが無効のため、GameObjectのSceneデータをデシリアライズできませんでした。");

		return false;
	}

	// コンポーネントのシーン情報のデシリアライズ
	if (!DeserializeSceneComponent(a_rootJson, a_componentSmartPointerVectorArray, a_gameObject)) { return false; }

	// 子ゲームオブジェクトのシーン情報のデシリアライズ
	if (const auto& l_childListJson = a_rootJson.value(Constant::k_gameObjectChildListJsonKey, nlohmann::json{});
		!l_childListJson.is_null()            &&
		Utility::IsJsonArray(l_childListJson) &&
		!DeserializeSceneChildList(l_childListJson, a_childDeserializeDataList, a_scene))
	{ 
		return false;
	}

	// シーンに接地されているプレハブで何番目かを格納
	a_gameObject.SetPrefabSceneInstanceNUM(l_prefabSceneInstanceNUM);

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

	return true;
}

nlohmann::json FWK::Converter::GameObjectSceneJsonConverter::Serialize(const GameObject& a_gameObject) const
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
	l_rootJson[k_prefabSceneInstanceNUMJsonKey] = l_prefabSceneInstanceNUM;
	l_rootJson[k_sceneInstanceNameJsonKey]      = l_sceneInstanceName;

	// Prefabを識別するためのUUID
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_prefabUUID, Constant::k_gameObjectPrefabUUIDJsonKey));

	// Scene上のGameObject自身を識別するUUID
	Utility::UpdateJson(l_rootJson, Utility::SerializeUUID(l_sceneInstanceUUID, k_sceneInstanceUUIDJsonKey));

	Utility::UpdateJson(l_rootJson, SerializeSceneComponent(a_gameObject));

	// 子ゲームオブジェクトの保存
	l_rootJson[Constant::k_gameObjectChildListJsonKey] = SerializeSceneChildList(a_gameObject);

	return l_rootJson;
}

bool FWK::Converter::GameObjectSceneJsonConverter::DeserializeSceneComponent(const nlohmann::json& a_rootJson, Utility::SmartPointerVectorArray<std::shared_ptr<ComponentBase>>& a_componentSmartPointerVectorArray, GameObject& a_gameObject) const
{
	const auto& l_transformComponent = a_gameObject.GetVALTransformComponent().lock();

	FWK_ASSERT_RETURN_VALUE_IF(!l_transformComponent, "TransformComponentが無効のため、GameObjectのSceneデータのデシリアライズに失敗しました。", false);

	// TransformComponentのプレハブの情報を読み込む
	const auto& l_transformComponentJson = a_rootJson.value(Constant::k_gameObjectTransformComponentJsonKey, nlohmann::json{});

	FWK_ASSERT_RETURN_VALUE_IF(l_transformComponentJson.is_null(), "TransformComponentJsonが無効のため、GameObjectのSceneデータのデシリアライズに失敗しました。", false);
	
	// TransformComponentのシーンデータをデシリアライズ
	l_transformComponent->DeserializeScene(l_transformComponentJson);

	// コンポーネント読み込み用Jonの存在確認、なければreturn
	if (const auto& l_componentJsonArray = a_rootJson.value(Constant::k_gameObjectComponentListJsonKey, nlohmann::json{});
		!l_componentJsonArray.is_null() &&
		Utility::IsJsonArray(l_componentJsonArray))
	{
		auto& l_componentList = a_componentSmartPointerVectorArray.GetMutableREFArrayElementDataList();;

		// コンポーネント数が一致しない場合コンポーネントのデシリアライズを行わない
		if (l_componentJsonArray.size() != l_componentList.size())
		{
			FWK_ADD_LOG("コンポーネントの数がPrefabとSceneで一致しないためComponentのScene情報のデシリアライズに失敗しました。");
		
			return false;
		}

		for (std::size_t l_i = 0U; l_i < l_componentJsonArray.size(); ++l_i)
		{
			const auto& l_json = l_componentJsonArray[l_i];

			if (l_json.is_null())
			{
				FWK_ADD_LOG("SceneのComponentListに無効なJsonが含まれています。");

				return false;
			}

			// 各コンポーネントのScene情報を読み込むJsonArrayで保存する関係上
			// 絶対に前回と同じコンポーネントの格納順番なので安全にインデックスを指定して
			// 読み込むことが可能
			const auto& l_component = l_componentList[l_i].m_type;

			if (!l_component) 
			{
				FWK_ADD_LOG("Prefabで生成したComponentが無効です。");

				return false;
			}

			l_component->DeserializeScene(l_json);
		}
	}

	return true;
}
bool FWK::Converter::GameObjectSceneJsonConverter::DeserializeSceneChildList(const nlohmann::json& a_rootJsonArray, std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, Scene& a_scene) const
{
	// リストが空なら読み込めていないのでreturn
	if (a_rootJsonArray.is_null()          ||
		!Utility::IsJsonArray(a_rootJsonArray))
	{
		return false; 
	}

	// Json配列のサイズが一致しなければreturn
	if (a_childDeserializeDataList.size() != a_rootJsonArray.size())
	{
		FWK_ADD_LOG("子の数がPrefabとSceneで一致しないためSceneの子情報のデシリアライズに失敗しました。");

		return false;
	}

	// 子の保存順序はJson::arrayで保証されているので安全にScene情報を読み取ることができる
	for (std::size_t l_i = 0ULL; l_i < a_rootJsonArray.size(); ++l_i)
	{
		const auto& l_json = a_rootJsonArray[l_i];

		if (l_json.is_null()) 
		{
			FWK_ADD_LOG("Prefab側に存在するChildGameObjectのSceneJsonが無効です。");

			return false;
		}

		auto& l_childDeserializeData = a_childDeserializeDataList[l_i];

		if (!l_childDeserializeData.m_self) { continue; }

		if (!l_childDeserializeData.m_self->DeserializeScene(l_json, 
			                                                 l_childDeserializeData.m_childDeserializeDataList, 
			                                                 l_childDeserializeData.m_componentSmartPointerVectorArray, 
			                                                 a_scene))
		{
			return false;
		}
	}

	return true;
}

nlohmann::json FWK::Converter::GameObjectSceneJsonConverter::SerializeSceneComponent(const GameObject& a_gameObject) const
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
		l_rootJson[Constant::k_gameObjectTransformComponentJsonKey] = l_json;
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
		l_json[Constant::k_gameObjectComponentTypeNameJsonKey] = l_component->GetREFRuntimeTypeINFO().k_name;
		
		Utility::UpdateJson(l_json, l_component->SerializeScene());

		// もし出力結果がnullならcontinue
		if (l_json.is_null()) { continue; }

		l_componentJsonArray.emplace_back(l_json);
	}

	// 自身を構成するコンポーネントを保存
	l_rootJson[Constant::k_gameObjectComponentListJsonKey] = l_componentJsonArray;

	return l_rootJson;
}
nlohmann::json FWK::Converter::GameObjectSceneJsonConverter::SerializeSceneChildList(const GameObject& a_gameObject) const
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