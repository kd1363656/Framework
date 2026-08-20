#include "ContentBrowserEditorWindowPrefabInstanceCreator.h"

std::weak_ptr<FWK::GameObject> FWK::Editor::ContentBrowserEditorWindowPrefabInstanceCreator::CreatePrefabInstance(const std::filesystem::path& a_prefabFilePath, const ContentBrowserEditorWindowAssetRegistry& a_assetRegistry, Scene& a_scene) const
{
	const auto& l_prefabUUID = a_assetRegistry.FindVALAssetUUID(a_prefabFilePath);

	if (l_prefabUUID.is_nil()) { return {}; }

	      auto& l_prefabSystem = a_scene.GetMutableREFPrefabSystem();
	const auto* l_prefab       = l_prefabSystem.FindPTRPrefab     (l_prefabUUID);

	if (!l_prefab) { return {}; }

	const auto& l_prefabJson = l_prefab->GetREFJson();

	if (l_prefabJson.is_null()) { return {}; }

	auto l_gameObject = std::make_shared<GameObject>();

	l_gameObject->INIT();

	std::vector<Struct::ChildDeserializeData> l_childDeserializeDataList = {};

	// PrefabJsonから
	// Root/Component/Childの構造を更新する
	if (!l_gameObject->DeserializePrefabInstance(l_prefabJson,
		                                         l_childDeserializeDataList,
		                                         a_scene))
	{
		return {};
	}

	// RootだけでなくChildPrefabにも
	// それぞれInstanceNUMを発行する
	if (!AllocatePrefabSceneInstanceNUMRecursive(l_gameObject, l_childDeserializeDataList, l_prefabSystem)) 
	{
		ReleasePrefabSceneInstanceNUMRecursive(l_gameObject, l_childDeserializeDataList, l_prefabSystem);

		return {}; 
	}

	a_scene.AddGameObject(l_gameObject);

	// PrefabInstanceNUMが割り当てられた子ゲームオブジェクトを再帰的に追加
	if (!l_gameObject->RecursiveAddChild(l_childDeserializeDataList, a_scene))
	{
		// 生成途中で親子関係構築に失敗した場合、
		// Allocate済みのPrefabSceneInstanceNUMをすべて返却する
		ReleasePrefabSceneInstanceNUMRecursive(l_gameObject, l_childDeserializeDataList, l_prefabSystem);

		l_gameObject->Destroy();

		return {};
	}

	// PostDeserializeでコンポーネント間の接続を行う
	RecursivePostDeserialize(l_gameObject);

	return l_gameObject;
}

void FWK::Editor::ContentBrowserEditorWindowPrefabInstanceCreator::RecursivePostDeserialize(const std::weak_ptr<GameObject>& a_gameObject) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return; }

	l_gameObject->PostDeserialize();

	const auto& l_childSmartPointerVectorArray = l_gameObject->GetREFChildSmartPointerVectorArray();

	for (const auto& l_childData : l_childSmartPointerVectorArray.GetREFArrayElementDataList())
	{
		const auto& l_child = l_childData.m_type;

		if (l_child.expired()) { continue; }

		RecursivePostDeserialize(l_child);
	}
}

bool FWK::Editor::ContentBrowserEditorWindowPrefabInstanceCreator::AllocatePrefabSceneInstanceNUMRecursive(const std::weak_ptr<GameObject>& a_gameObject, const std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, PrefabSystem& a_prefabSystem) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return false; }

	const auto& l_prefabUUID = l_gameObject->GetREFPrefabUUID();

	if (l_prefabUUID.is_nil()) { return false; }

	const auto l_prefabSceneInstanceNUM = a_prefabSystem.AllocatePrefabInstanceNUM(l_prefabUUID);

	if (l_prefabSceneInstanceNUM == Constant::k_invalidPrefabSceneInstanceNUM) { return false; }
	
	l_gameObject->SetPrefabSceneInstanceNUM(l_prefabSceneInstanceNUM);

	for (const auto& l_childDeserializeData : a_childDeserializeDataList)
	{
		if (!l_childDeserializeData.m_self) { continue; }

		if (!AllocatePrefabSceneInstanceNUMRecursive(l_childDeserializeData.m_self, l_childDeserializeData.m_childDeserializeDataList, a_prefabSystem)) { return false; }
	}

	return true;
}

void FWK::Editor::ContentBrowserEditorWindowPrefabInstanceCreator::ReleasePrefabSceneInstanceNUMRecursive(const std::weak_ptr<GameObject>& a_gameObject, const std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, PrefabSystem& a_prefabSystem) const
{
	const auto& l_gameObject = a_gameObject.lock();

	if (!l_gameObject) { return; }

	const auto& l_prefabUUID             = l_gameObject->GetREFPrefabUUID            ();
	const auto  l_prefabSceneInstanceNUM = l_gameObject->GetVALPrefabSceneInstanceNUM();

	// Allocate済みGameOBjectだけをAllocatorへ返却する
	if (!l_prefabUUID.is_nil() &&
		l_prefabSceneInstanceNUM != Constant::k_invalidPrefabSceneInstanceNUM)
	{
		a_prefabSystem.ReleasePrefabInstanceNUM(l_prefabUUID, l_prefabSceneInstanceNUM);

		// 同じGameObjectが後で破棄処理されたとしても
		// 二重Releaseされないよう無効値へ戻す
		l_gameObject->SetPrefabSceneInstanceNUM(Constant::k_invalidPrefabSceneInstanceNUM);
	}

	// Child側にも既にAllocate済みのものが存在する可能性があるので、
	// すべて再帰的に確認する
	for (const auto& l_childDeserilaizeData : a_childDeserializeDataList)
	{
		if (!l_childDeserilaizeData.m_self) { continue; }

		ReleasePrefabSceneInstanceNUMRecursive(l_childDeserilaizeData.m_self, l_childDeserilaizeData.m_childDeserializeDataList, a_prefabSystem);
	}

	return;
}