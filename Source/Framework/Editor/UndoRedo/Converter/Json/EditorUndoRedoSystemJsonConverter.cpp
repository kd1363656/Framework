#include "EditorUndoRedoSystemJsonConverter.h"

void FWK::Converter::EditorUndoRedoSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::EditorUndoRedoSystem& a_editorUndoRedoSystem) const
{
    if (a_rootJson.is_null()) { return; }

    const auto l_capacity = a_rootJson.value(k_capacityJsonKey, Constant::k_initialEditorUndoResoSystemListCapacity);

    a_editorUndoRedoSystem.SetCapacity(l_capacity);
}

nlohmann::json FWK::Converter::EditorUndoRedoSystemJsonConverter::Serialize(const Editor::EditorUndoRedoSystem& a_editorUndoRedoSystem) const
{
    nlohmann::json l_rootJson = {};

    l_rootJson[k_capacityJsonKey] = a_editorUndoRedoSystem.GetREFCapacity();

    return l_rootJson;
}