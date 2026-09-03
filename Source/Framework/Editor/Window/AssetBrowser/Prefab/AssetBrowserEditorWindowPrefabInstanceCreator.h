#pragma once

namespace FWK::Editor
{
	class AssetBrowserEditorWindowPrefabInstanceCreator final
	{
	public:

		 AssetBrowserEditorWindowPrefabInstanceCreator() = default;
		~AssetBrowserEditorWindowPrefabInstanceCreator() = default;

		std::weak_ptr<GameObject> CreatePrefabInstance(const AssetFilePathRegistry& a_assetFilePathRegistry, const std::filesystem::path& a_prefabFilePath, Scene& a_scene) const;

	private:

		const Prefab* SynchronizePrefabIfNotExist(const AssetFilePathRegistry& a_assetFilePathRegistry, const std::filesystem::path& a_prefabFilePath, PrefabSystem& a_prefabSystem) const;

		void RecursivePostDeserialize(const std::weak_ptr<GameObject>& a_gameObject) const;

		bool AllocatePrefabSceneInstanceNUMRecursive(const std::weak_ptr<GameObject>& a_gameObject, const std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, PrefabSystem& a_prefabSystem) const;

		void ReleasePrefabSceneInstanceNUMRecursive(const std::weak_ptr<GameObject>& a_gameObject, const std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, PrefabSystem& a_prefabSystem) const;
	};
}