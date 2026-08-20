#pragma once

namespace FWK::Editor
{
	class ContentBrowserEditorWindowPrefabInstanceCreator final
	{
	public:

		 ContentBrowserEditorWindowPrefabInstanceCreator() = default;
		~ContentBrowserEditorWindowPrefabInstanceCreator() = default;

		std::weak_ptr<GameObject> CreatePrefabInstance(const std::filesystem::path& a_prefabFilePath, const ContentBrowserEditorWindowAssetRegistry& a_assetRegistry, Scene& a_scene) const;

	private:

		void RecursivePostDeserialize(const std::weak_ptr<GameObject>& a_gameObject) const;

		bool AllocatePrefabSceneInstanceNUMRecursive(const std::weak_ptr<GameObject>& a_gameObject, const std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, PrefabSystem& a_prefabSystem) const;

		void ReleasePrefabSceneInstanceNUMRecursive(const std::weak_ptr<GameObject>& a_gameObject, const std::vector<Struct::ChildDeserializeData>& a_childDeserializeDataList, PrefabSystem& a_prefabSystem) const;
	};
}