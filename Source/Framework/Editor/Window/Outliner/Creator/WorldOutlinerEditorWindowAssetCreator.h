#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowAssetCreator final
    {
    public:

         WorldOutlinerEditorWindowAssetCreator() = default;
        ~WorldOutlinerEditorWindowAssetCreator() = default;

        std::shared_ptr<GameObject> CreateEmptyGameObject(const std::weak_ptr<GameObject>& a_parent, Scene& a_scene) const;

        std::shared_ptr<GameObject> CreateGameObjectFromPrefab(const std::weak_ptr<GameObject>& a_parent, const std::filesystem::path& a_prefabFilePath, Scene& a_scene) const;

        // ドロップされたファイルパスのうちPrefabとして登録されている物だけを生成対象にする
        std::vector<std::weak_ptr<GameObject>> CreateGameObjectFromPrefabDrop(const std::vector<std::filesystem::path>& a_droppedFilePathList, const std::weak_ptr<GameObject>& a_parent, Scene& a_scene) const;
    };
}