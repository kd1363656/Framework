#include "EditorWindowPaneSplitterJsonConverter.h"

void FWK::Converter::EditorWindowPaneSplitterJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::EditorWindowPaneSplitter& a_editorWindowPaneSplitter) const
{
    if (a_rootJson.is_null()) { return; }

    const float l_primaryPaneSize = a_rootJson.value(k_primaryPaneSizeJsonKey, Constant::k_imguiDefaultPrimaryPaneSize);

    a_editorWindowPaneSplitter.SetPrimaryPaneSize(l_primaryPaneSize);
}

nlohmann::json FWK::Converter::EditorWindowPaneSplitterJsonConverter::Serialize(const Editor::EditorWindowPaneSplitter& a_editorWindowPaneSplitter) const
{
    nlohmann::json l_rootJson = {};

    const float l_primaryPaneSize = a_editorWindowPaneSplitter.GetVALPrimaryPaneSize();

    l_rootJson[k_primaryPaneSizeJsonKey] = l_primaryPaneSize;

    return l_rootJson;
}