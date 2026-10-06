#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowGameObjectOperation final
    {
    private:

        using ChildGameObjectDataList = std::vector<Struct::SmartPointerVectorListArrayElementData<std::weak_ptr<GameObject>>>;

    public:

         WorldOutlinerEditorWindowGameObjectOperation() = default;
        ~WorldOutlinerEditorWindowGameObjectOperation() = default;

        void RenameGameObject          (const std::weak_ptr<GameObject>&      a_gameObject, const std::string& a_newName)                                            const;
        void DestroySelectedGameObjects(      EditorGameObjectSelectionState& a_gameObjectSelectionState)                                                            const;
        void ReparentGameObject        (const std::weak_ptr<GameObject>&      a_targetGameObject, const std::weak_ptr<GameObject>& a_moveGameObject, Scene& a_scene) const;

        void MoveGameObjectSiblingOrder(const std::weak_ptr<GameObject>& a_targetGameObject,
                                        const std::weak_ptr<GameObject>& a_moveGameObject,
                                        const bool                       a_isMoveAfter,
                                              Scene&                     a_scene) const;

        void CopySelectedGameObjects     (const EditorGameObjectSelectionState&     a_gameObjectSelectionState, WorldOutlinerEditorWindowClipboard& a_clipboard)                                const;
        void CutSelectedGameObjects      (      WorldOutlinerEditorWindowClipboard& a_clipboard,                EditorGameObjectSelectionState&     a_gameObjectSelectionState)                 const;
        void DuplicateSelectedGameObjects(      EditorGameObjectSelectionState&     a_gameObjectSelectionState, Scene&                              a_scene)                                    const;
        void PasteGameObjects            (      WorldOutlinerEditorWindowClipboard& a_clipboard,                EditorGameObjectSelectionState&     a_gameObjectSelectionState, Scene& a_scene) const;

    private:
    
        static constexpr std::size_t k_rootPasteParentCount = 1ULL;

        void DestroyGameObjectRecursive(const std::weak_ptr<GameObject>& a_gameObject) const;

        ChildGameObjectDataList::iterator FindChildGameObjectITR(const std::weak_ptr<GameObject>& a_gameObject, ChildGameObjectDataList& a_childDataList) const;
    };
}