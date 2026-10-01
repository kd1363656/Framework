#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowGameObjectOperation final
    {
    private:

        using ChildGameObjectDataList = std::vector<Utility::SmartPointerVectorList<std::weak_ptr<GameObject>>::ArrayElementData>;

    public:

         WorldOutlinerEditorWindowGameObjectOperation() = default;
        ~WorldOutlinerEditorWindowGameObjectOperation() = default;

        void RenameGameObject          (const std::weak_ptr<GameObject>&      a_gameObject, const std::string& a_newName) const;
        void DestroySelectedGameObjects(      EditorGameObjectSelectionState& a_gameObjectSelectionState) const;
        void ReparentGameObject        (const std::weak_ptr<GameObject>&      a_targetGameObject, const std::weak_ptr<GameObject>& a_moveGameObject, Scene& a_scene);

        void MoveGameObjectSiblingOrder(const std::weak_ptr<GameObject>& a_targetGameObject, 
                                        const std::weak_ptr<GameObject>& a_moveGameObject, 
                                              Scene&                     a_scene,
                                        const bool                       a_isMoveAfter) const;

        void CopySelectedGameObjects     (const EditorGameObjectSelectionState&     a_gameObjectSelectionState, WorldOutlinerEditorWindowClipboard& a_clipboard) const;
        void CutSelectedGameObjects      (      WorldOutlinerEditorWindowClipboard& a_clipboard,                EditorGameObjectSelectionState&     a_gameObjectSelectionState) const;
        void DuplicateSelectedGameObjects(      EditorGameObjectSelectionState&     a_gameObjectSelectionState, Scene&                              a_scene) const;
        void PasteGameObjects            (      WorldOutlinerEditorWindowClipboard& a_clipboard,                EditorGameObjectSelectionState&     a_gameObjectSelectionState, Scene& a_scene) const;

    private:
    
        // 指定GameObjectとその子孫を再帰的に破棄する
        void DestroyGameObjectRecursive(const std::weak_ptr<GameObject>& a_gameObject) const;

        // 選択リスト内に祖先が含まれているか判定する(親子同時選択時の重複Clone回避用)
        bool HasAncestorInSelection(const std::vector<std::weak_ptr<GameObject>>& a_selectedList, const std::weak_ptr<GameObject>& a_gameObject) const;

        ChildGameObjectDataList::iterator FindChildGameObjectITR(const std::weak_ptr<GameObject>& a_gameObject, ChildGameObjectDataList& a_childDataList) const;
    };
}