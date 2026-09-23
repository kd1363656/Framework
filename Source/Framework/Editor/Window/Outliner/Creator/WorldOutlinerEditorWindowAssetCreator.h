#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowAssetCreator final
    {
    public:

         WorldOutlinerEditorWindowAssetCreator() = default;
        ~WorldOutlinerEditorWindowAssetCreator() = default;
    
        std::weak_ptr<GameObject> CreateEmptyGameObject(Scene& a_scene) const;

        std::vector<boost::uuids::uuid> DuplicateGameObjects(const std::vector<boost::uuids::uuid>& a_selectedUUIDList, Scene& a_scene) const;

    private:
    
        std::shared_ptr<GameObject> DuplicateGameObjectRecursive(const GameObject& a_source, Scene& a_scene) const;
    };
}