#pragma once

namespace FWK::Editor
{
    class EditorManager;
}

namespace FWK::Editor
{
    class EditorWindowBase
    {
    public:

                 EditorWindowBase() = default;
        virtual ~EditorWindowBase() = default;

        virtual void Deserialize    (const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };
        virtual void PostDeserialize()                      { /*必要に応じてオーバーライドしてください*/ };

        virtual void Draw(EditorManager& a_editorManager) = 0;

        virtual nlohmann::json Serialize() { return nlohmann::json{}; }

        // エディターを描画しない(Ctrl + Pで隠している)フレームに呼ばれる
        virtual void OnDrawSkipped() { /*必要に応じてオーバーライドしてください*/ };

    protected:

        void ReportActiveWindowIfMouseClicked(EditorManager& a_editorManager) const;

        FWK_DEFINE_TYPE_INFO_ROOT(EditorWindowBase)
    };
}