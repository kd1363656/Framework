#include "NextSceneJsonConverter.h"

bool FWK::Converter::NextSceneJsonConverter::Rename(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const std::string& a_newName)
{
    // NextScene::Load + Saveを使わず
    // JSONを直接読み込んでNextSceneキーだけ書き換える
    if (a_oldFilePath.empty() ||
        a_oldFilePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "NextSceneのRename元FilePathが無効です。\nOldFilePath : {}", a_oldFilePath.string());

        return false;
    }

    if (a_newFilePath.empty() ||
        a_newFilePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "NextSceneのRename先FilePathが無効です。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    if (a_newName.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "NextSceneの新しい名前が空のため、Renameを中止しました。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    // プレハブJSONからNextScene情報を読み込む
    auto l_rootJson = Utility::LoadJsonFile(a_oldFilePath);

    if (l_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "NextSceneの読み込みが失敗したため、Renameを中止しました。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    // 読み込んだファイルの名前部分だけ変更
    l_rootJson[Constant::k_sceneJsonConverterNameJsonKey] = a_newName;

    Utility::SaveJsonFile(l_rootJson, a_newFilePath);

    return true;
}

void FWK::Converter::NextSceneJsonConverter::Load(nlohmann::json&& a_rootJson, NextScene& a_nextScene) const
{
    if (a_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "RootJsonが無効となっており、NextSceneのデシリアライズに失敗しました。");

        return;
    }

    const auto& l_name = a_rootJson.value(Constant::k_sceneJsonConverterNameJsonKey, std::string{});

    // NextSceneNameは、このNextSceneから生成される
    if (l_name.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "NextSceneのNameが空となっており、NextSceneのデシリアライズに失敗しました。");

        return;
    }

    a_nextScene.SetName(l_name);
    a_nextScene.SetJson(std::move(a_rootJson));
}