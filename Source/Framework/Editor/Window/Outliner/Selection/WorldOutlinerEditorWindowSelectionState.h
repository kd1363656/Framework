#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowSelectionState final : public std::enable_shared_from_this<WorldOutlinerEditorWindowSelectionState>
    {
    public:

         WorldOutlinerEditorWindowSelectionState() = default;
        ~WorldOutlinerEditorWindowSelectionState() = default;
    
        void SelectSingleScene();

        void SetIsSceneSelected(const bool a_set) { m_isSceneSelected = a_set; }

        bool GetVALIsSceneSelected() const { return m_isSceneSelected; }

    private:
    
        bool m_isSceneSelected = false;
    };
}