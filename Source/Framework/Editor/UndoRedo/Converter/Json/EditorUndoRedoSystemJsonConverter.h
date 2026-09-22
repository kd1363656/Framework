#pragma once

namespace FWK::Editor
{
    class EditorUndoRedoSystem;
}

namespace FWK::Converter
{
    class EditorUndoRedoSystemJsonConverter final
    {
    public:

         EditorUndoRedoSystemJsonConverter() = default;
        ~EditorUndoRedoSystemJsonConverter() = default;

        void Deserialize(const nlohmann::json& a_rootJson, Editor::EditorUndoRedoSystem& a_editorUndoRedoSystem) const;

        nlohmann::json Serialize(const Editor::EditorUndoRedoSystem& a_editorUndoRedoSystem) const;

    private:

        static constexpr std::string_view k_capacityJsonKey = "CapacityJsonKey";
    };
}