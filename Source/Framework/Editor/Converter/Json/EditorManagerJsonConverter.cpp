#include "EditorManagerJsonConverter.h"

void FWK::Converter::EditorManagerJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Editor::EditorManager& a_editorManager) const
{
	if (a_rootJson.is_null()) { return; }

	if (a_rootJson.contains(k_editorWindowListJsonKey))
	{
		DeserializeEditorWindow(a_rootJson[k_editorWindowListJsonKey], a_editorManager);
	}
}

nlohmann::json FWK::Converter::EditorManagerJsonConverter::Serialize(const Editor::EditorManager& a_editorManager) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_editorWindowListJsonKey] = SerializeEditorWindow(a_editorManager);

	return l_rootJson;
}

void FWK::Converter::EditorManagerJsonConverter::DeserializeEditorWindow(const nlohmann::json& a_rootJson, Editor::EditorManager& a_editorManager) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	// jsonファイルに保存されていたエディターを復元
	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Editor::EditorWindowBase> l_editorWindow = nullptr;
		
		Utility::DeserializeInstanceType<TypeAlias::SharedFactoryEditorWindow>(l_json, k_editorWindowJsonKey, l_editorWindow);

		if (!l_editorWindow) { continue; }

		a_editorManager.AddEditorWindow(l_editorWindow);
	}
}

nlohmann::json FWK::Converter::EditorManagerJsonConverter::SerializeEditorWindow(const Editor::EditorManager& a_editorManager) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	// 現在使用しているエディターの情報をjsonファイルに保存
	for (const auto& l_editorWindow : a_editorManager.GetREFEditorWindowList())
	{
		if (!l_editorWindow) { continue; }

		l_rootJsonArray.emplace_back(Utility::SerializeInstanceType(l_editorWindow, k_editorWindowJsonKey));
	}

	return l_rootJsonArray;
}