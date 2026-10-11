#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowGameObjectOperation final
    {
    public:

         WorldOutlinerEditorWindowGameObjectOperation() = default;
        ~WorldOutlinerEditorWindowGameObjectOperation() = default;

        void RenameGameObject          (const std::weak_ptr<GameObject>&      a_gameObject, const std::string& a_newName)                                            const;
        void DestroySelectedGameObjects(      EditorGameObjectSelectionState& a_gameObjectSelectionState, Scene& a_scene)                                            const;
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

        void PushReparentGameObjectCommand(const std::weak_ptr<GameObject>& a_gameObject, const Scene& a_scene, const Struct::ReparentGameObjectState& a_beforeState) const;

        Struct::DestroyedGameObjectRecord FetchVALDestroyedGameObjectRecord(const std::weak_ptr<GameObject>& a_gameObject) const;

        std::vector<Struct::SmartPointerVectorListArrayElementData<std::weak_ptr<GameObject>>>::iterator FindChildGameObjectITR(const std::weak_ptr<GameObject>& a_gameObject, std::vector<Struct::SmartPointerVectorListArrayElementData<std::weak_ptr<GameObject>>>& a_childDataList) const;

        static constexpr std::size_t k_rootPasteParentCount = 1ULL;
    };
}