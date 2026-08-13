#include "GameObjectJsonConverter.h"

void FWK::GameObjectJsonConverter::Deserialize(const std::weak_ptr<GameObject>&              a_gameObject, 
	                                           const nlohmann::json&                         a_rootJson,  
	                                                 std::unordered_set<boost::uuids::uuid>& a_prefabUUIDSet,
	                                                 Scene&                                  a_scene) const
{
	if (a_rootJson.is_null()) { return; }

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
		l_prefab->SetGameObject(l_prefab->GetREFGameObject());
	}

	const auto& l_prefabJson = l_prefab->GetREFJson();

	if (!l_prefabJson.is_null()) 
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

		// 子のプレハブUUIDを読み込む
		const auto& l_prefabUUID = Utility::DeserializeUUID(l_childJson, k_prefabUUIDJsonKey);
		
		if (l_prefabUUID.is_nil())
		{
			FWK_ADD_LOG("子GameObjectのPrefabUUIDが無効となっており、子ゲームオブジェクトをデシリアライズできませんでした。");

			continue;
		}

		// emplaceの戻り値を使うことで
		// contains()とemplace()による二重検索を避ける
		// 登録できなかった場合は、現在の親階層に
		// 同じPrefab名が既に存在している
		if (!a_prefabUUIDSet.emplace(l_prefabUUID).second)
		{
			FWK_ADD_LOG("親階層と同じPrefabUUIDを持つ子GameObjectを検出したため、子ゲームオブジェクトのデシリアライズを除外しました。");

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
		a_prefabUUIDSet.erase(l_prefabUUID);

		// 構造体にSceneデシリアライズ用に自身を格納
		l_childDeserializeData.m_self = l_child;
	}

	return true;
}