#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class EditorGameObjectSelectionState final
    {
    public:

         EditorGameObjectSelectionState() = default;
        ~EditorGameObjectSelectionState() = default;

        void SelectSingleGameObject(const std::weak_ptr<GameObject>&              a_gameObject);
        void SelectGameObjectRange (const std::vector<std::weak_ptr<GameObject>>& a_gameObjectList);

        void ToggleSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject);

        void AddSelectedGameObject   (const std::weak_ptr<GameObject>& a_gameObject);
        void RemoveSelectedGameObject(const std::weak_ptr<GameObject>& a_gameObject);

        void ClearSelectedGameObjectList();

        void SweepUnavailableGameObjects();

        void RestoreState(const std::vector<boost::uuids::uuid>& a_uuidList, const boost::uuids::uuid& a_anchorUUID);

        const auto& GetREFSelectedGameObjectList() const { return m_selectedGameObjectList; }
        const auto& GetREFRangeSelectionAnchor  () const { return m_rangeSelectionAnchor;    }

        std::weak_ptr<GameObject> FindVALLastSelectedGameObject() const;

        bool FindVALIsSelected(const std::weak_ptr<GameObject>& a_gameObject) const;

    private:

        std::vector<std::weak_ptr<GameObject>> m_selectedGameObjectList = {};

        std::weak_ptr<GameObject> m_rangeSelectionAnchor = {};
    };
}