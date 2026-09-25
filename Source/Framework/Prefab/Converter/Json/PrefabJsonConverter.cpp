#include "PrefabJsonConverter.h"

bool FWK::Converter::PrefabJsonConverter::Rename(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const std::string& a_newName)
{
    // Prefab::Load + Saveを使わず
    // JSONを直接読み込んでPrefabNameキーだけ書き換える
    if (a_oldFilePath.empty() ||
        a_oldFilePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabのRename元FilePathが無効です。\nOldFilePath : {}", a_oldFilePath.string());

        return false;
    }

    if (a_newFilePath.empty() ||
        a_newFilePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabのRename先FilePathが無効です。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    if (a_newName.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefabの新しい名前が空のため、Renameを中止しました。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    // プレハブJSONからPrefab情報を読み込む
    auto l_rootJson = Utility::LoadJsonFile(a_oldFilePath);

    if (l_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "Prefabの読み込みが失敗したため、Renameを中止しました。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    // 読み込んだファイルの名前部分だけ変更
    l_rootJson[k_prefabNameJsonKey] = a_newName;

    Utility::SaveJsonFile(l_rootJson, a_newFilePath);

    return true;
}

void FWK::Converter::PrefabJsonConverter::Load(const nlohmann::json& a_rootJson, Prefab& a_prefab) const
{
    if (a_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "RootJsonが無効となっており、Prefabのデシリアライズに失敗しました。");

        return;
    }

    const auto& l_prefabName = a_rootJson.value(k_prefabNameJsonKey, std::string{});

    // PrefabNameは、このPrefabから生成される
    // GameObject名の元になるための必須情報
    if (l_prefabName.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabNameが空となっており、Prefabのデシリアライズに失敗しました。");

        return;
    }

    a_prefab.SetPrefabName(l_prefabName);

    const auto& l_json = a_rootJson.value(k_prefabJsonKey, nlohmann::json{});

    if (l_json.is_null()) { return; }

    a_prefab.SetJson(l_json);
}

bool FWK::Converter::PrefabJsonConverter::Save(const std::filesystem::path& a_filePath, Prefab& a_prefab) const
{
    // 読み込めるファイルでなければ保存しない
    if (a_filePath.empty() ||
        a_filePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "書き込みが不可能なファイルパスになっており、書き込み処理に失敗しました。");

        return false;
    }

    // TODO
    nlohmann::json l_rootJson = {};

    l_rootJson[k_prefabNameJsonKey] = a_prefab.GetREFPrefabName();

    // ファイル書き込みに失敗した場合は
    // Prefab内部のキャッシュも更新しない
    if (!Utility::SaveJsonFile(l_rootJson, a_filePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "PrefabファイルへのJson書き込みに失敗しました。\nFilePath : {}", a_filePath.string());

        return false;
    }

    return true;
}