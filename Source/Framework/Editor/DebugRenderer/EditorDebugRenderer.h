#pragma once

namespace FWK::Editor
{
    class EditorDebugRenderer final
    {
    public:

         EditorDebugRenderer() = default;
        ~EditorDebugRenderer() = default;

        void CollectDebugDrawCommands(const EditorGameObjectSelectionState& a_gameObjectSelectionState, const bool a_isDrawFrustum);

        const auto& GetREFEditorDebugRendererQueue() const { return m_editorDebugRendererQueue; }

    private:

        void CollectSelectedCameraFrustum(const EditorGameObjectSelectionState& a_gameObjectSelectionState);

        EditorDebugRendererQueue m_editorDebugRendererQueue = {};
    };
}