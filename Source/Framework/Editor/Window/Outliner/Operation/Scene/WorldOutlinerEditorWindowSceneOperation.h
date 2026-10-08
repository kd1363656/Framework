#pragma once

namespace FWK::Editor
{
    class WorldOutlinerEditorWindowSceneOperation final
    {
    public:

         WorldOutlinerEditorWindowSceneOperation() = default;
        ~WorldOutlinerEditorWindowSceneOperation() = default;

        void RenameScene(const std::string& a_newName, Scene& a_scene) const;
    };
}