#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowAssetCreator final
    {
    public:

         WorldOutlinerEditorWindowAssetCreator() = default;
        ~WorldOutlinerEditorWindowAssetCreator() = default;
    
        std::shared_ptr<GameObject> CreateEmptyGameObject(const std::weak_ptr<GameObject>& a_parent, Scene& a_scene) const;
    };
}