#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowGameObjectOperation final
    {
    public:

         WorldOutlinerEditorWindowGameObjectOperation() = default;
        ~WorldOutlinerEditorWindowGameObjectOperation() = default;

        void RenameGameObject          (const std::shared_ptr<GameObject>&    a_gameObject, const std::string& a_newName) const;
        void DestroySelectedGameObjects(      EditorGameObjectSelectionState& a_gameObjectSelectionState) const;

    private:
    
        // 指定GameObjectとその子孫を再帰的に破棄する
        void DestroyGameObjectRecursive(const std::shared_ptr<GameObject>& a_gameObject) const;
    };
}