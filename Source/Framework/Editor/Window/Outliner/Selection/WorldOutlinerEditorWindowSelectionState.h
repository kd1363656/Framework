#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowSelectionState final : public std::enable_shared_from_this<WorldOutlinerEditorWindowSelectionState>
    {
    public:

         WorldOutlinerEditorWindowSelectionState() = default;
        ~WorldOutlinerEditorWindowSelectionState() = default;
    
        void SelectSingleGameObject(const std::weak_ptr<GameObject>& a_gameObject);
        void SelectSingleScene     ();

        void ClearSelection();

        void SetSelectedGameObjectUUIDList(const std::vector<boost::uuids::uuid>& a_set) { m_selectedGameObjectUUIDList = a_set; }

        void SetIsSceneSelected(const bool a_set) { m_isSceneSelected = a_set; }

        const auto& GetREFSelectedGameObjectUUIDList() const { return m_selectedGameObjectUUIDList; }

        bool GetVALIsSceneSelected() const { return m_isSceneSelected; }

    private:
    
        std::vector<boost::uuids::uuid> m_selectedGameObjectUUIDList = {};

        bool m_isSceneSelected = false;
    };
}