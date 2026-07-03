#include "EditorManagerJsonConverter.h"

void FWK::Converter::EditorManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::EditorManager& a_editorManager) const
{
	if (a_rootJson.is_null()) { return; }

	// エディターウィンドウリストのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_windowListJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeWindow(l_json, a_editorManager);
	}

	// メインメニューバーのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_mainMenubarJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_mainMenubar = a_editorManager.GetMutableREFMainMenubar();

		l_mainMenubar.Deserialize(l_json);
	}

	const bool l_isDisableDrawEditor = a_rootJson.value(k_isDisableDrawJsonKey, false);

	a_editorManager.SetIsDisableDrawEditor(l_isDisableDrawEditor);
}

nlohmann::json FWK::Converter::EditorManagerJsonConverter::Serialize(const Editor::EditorManager& a_editorManager) const
{
	nlohmann::json l_rootJson = {};

	const auto& l_mainMenubar = a_editorManager.GetREFMainMenubar();

	// エディターウィンドウリストのシリアライズ
	l_rootJson[k_windowListJsonKey] = SerializeWindow(a_editorManager);

	// メインメニューバーのシリアライズ
	l_rootJson[k_mainMenubarJsonKey] = l_mainMenubar.Serialize();

	l_rootJson[k_isDisableDrawJsonKey] = a_editorManager.GetVALIsDisableDrawEditor();

	return l_rootJson;
}

void FWK::Converter::EditorManagerJsonConverter::DeserializeWindow(const nlohmann::json& a_rootJson, Editor::EditorManager& a_editorManager) const
{
	if (a_rootJson.is_null())		       { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }

	// jsonファイルに保存されていたエディターを復元
	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Editor::EditorWindowBase> l_editorWindow = nullptr;
		
		Utility::DeserializeInstanceType<TypeAlias::EditorWindowSharedFactory>(l_json, k_windowJsonKey, l_editorWindow);

		if (!l_editorWindow) { continue; }

		a_editorManager.AddEditorWindow(l_editorWindow);
	}
}

nlohmann::json FWK::Converter::EditorManagerJsonConverter::SerializeWindow(const Editor::EditorManager& a_editorManager) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	// 現在使用しているエディターの情報をjsonファイルに保存
	for (const auto& l_editorWindow : a_editorManager.GetREFEditorWindowList())
	{
		if (!l_editorWindow) { continue; }

		l_rootJsonArray.emplace_back(Utility::SerializeInstanceType(l_editorWindow, k_windowJsonKey));
	}

	return l_rootJsonArray;
}