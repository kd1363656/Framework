#include "MainMenubarEditorJsonConverter.h"

void FWK::Converter::MainMenubarEditorJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::MainMenubarEditor& a_mainMenubarEditor) const
{
    // メインメニューのデシリアライズ
    if (const auto& l_json = a_rootJson.value(k_mainMenuListJsonKey, nlohmann::json{});
        !l_json.is_null())
    {
        DeserializeEditorMainMenu(l_json, a_mainMenubarEditor);
    }
}

nlohmann::json FWK::Converter::MainMenubarEditorJsonConverter::Serialize(const Editor::MainMenubarEditor& a_mainMenubarEditor) const
{
    nlohmann::json l_rootJson = {};

    // メインメニューのシリアライズ
    l_rootJson[k_mainMenuListJsonKey] = SerializeEditormainMenu(a_mainMenubarEditor);

    return l_rootJson;
}

void FWK::Converter::MainMenubarEditorJsonConverter::DeserializeEditorMainMenu(const nlohmann::json& a_rootJson, Editor::MainMenubarEditor& a_mainMenubarEditor) const
{
    if (a_rootJson.is_null())              { return; }
    if (!Utility::IsJsonArray(a_rootJson)) { return; }

    for (const auto& l_json : a_rootJson)
    {
        std::unique_ptr<Editor::EditorMainMenuBase> l_editorMainMenuBase = nullptr;

        Utility::DeserializeInstanceType<TypeAlias::EditorMainMenuUniqueFactory>(l_json, k_mainMenuTypeName, l_editorMainMenuBase);

        if (!l_editorMainMenuBase) 
        {
            FWK_ADD_LOG(Constant::k_debugWarningColor, "EditorMainMenuが復元されておらず、追加処理に失敗しました。");
            continue;
        }

        a_mainMenubarEditor.AddEditorMainMenu(std::move(l_editorMainMenuBase));
    }
}

nlohmann::json FWK::Converter::MainMenubarEditorJsonConverter::SerializeEditormainMenu(const Editor::MainMenubarEditor& a_mainMenubarEditor) const
{
    nlohmann::json l_rootJsonArray = {};

    const auto& l_editorMainMenuSmartPointerVectorArray = a_mainMenubarEditor.GetREFEditorMainMenuSmartPointerVectorArray();

    for (const auto& l_editorMainMenuData : l_editorMainMenuSmartPointerVectorArray.GetREFArrayElementDataList())
    {
        const auto& l_editorMainMenu = l_editorMainMenuData.m_type;

        if (!l_editorMainMenu) { continue; }

        nlohmann::json l_json = {};

        Utility::UpdateJson(l_json, Utility::SerializeInstanceType(l_editorMainMenu, k_mainMenuTypeName));

        l_rootJsonArray.emplace_back(l_json);
    }

    return l_rootJsonArray;
}