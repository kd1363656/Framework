#include "GameObjectPrefabJsonConverter.h"

bool FWK::Converter::GameObjectPrefabJsonConverter::Rename(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const std::string& a_newName)
{
    // GameObjectPrefab::Load + Saveを使わず
    // JSONを直接読み込んでGameObjectPrefabNameキーだけ書き換える
    if (a_oldFilePath.empty() ||
        a_oldFilePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabのRename元FilePathが無効です。\nOldFilePath : {}", a_oldFilePath.string());

        return false;
    }

    if (a_newFilePath.empty() ||
        a_newFilePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabのRename先FilePathが無効です。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    if (a_newName.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabの新しい名前が空のため、Renameを中止しました。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    // プレハブJSONからGameObjectPrefab情報を読み込む
    auto l_rootJson = Utility::LoadJsonFile(a_oldFilePath);

    if (l_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabの読み込みが失敗したため、Renameを中止しました。\nNewFilePath : {}", a_newFilePath.string());

        return false;
    }

    // 読み込んだファイルの名前部分だけ変更
    l_rootJson[Constant::k_gameObjectJsonConverterNameJsonKey] = a_newName;

    Utility::SaveJsonFile(l_rootJson, a_newFilePath);

    return true;
}

void FWK::Converter::GameObjectPrefabJsonConverter::Load(const nlohmann::json& a_rootJson, GameObjectPrefab& a_prefab) const
{
    if (a_rootJson.is_null())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "RootJsonが無効となっており、GameObjectPrefabのデシリアライズに失敗しました。");

        return;
    }

    const auto& l_name = a_rootJson.value(Constant::k_gameObjectJsonConverterNameJsonKey, std::string{});

    // GameObjectPrefabNameは、このGameObjectPrefabから生成される
    // GameObject名の元になるための必須情報
    if (l_name.empty())
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabNameが空となっており、GameObjectPrefabのデシリアライズに失敗しました。");

        return;
    }

    a_prefab.SetName(l_name);

    auto l_json = a_rootJson.value(k_prefabJsonKey, nlohmann::json{});

    if (l_json.is_null()) { return; }

    // SceneInstanceUUIDはシーン内でのみ有効な識別子でPrefabが持つべきではない
    // 旧形式で保存されていたPrefabファイルに残っていてもここで取り除く
    RemoveSceneInstanceUUIDRecursively(l_json);

    a_prefab.SetJson(std::move(l_json));
}

bool FWK::Converter::GameObjectPrefabJsonConverter::Save(const std::filesystem::path&       a_filePath, 
                                                         const GameObject&                  a_gameObject,
                                                               SceneGameObjectPrefabSystem& a_prefabSystem, 
                                                               GameObjectPrefab&            a_prefab) const
{
    // 読み込めるファイルでなければ保存しない
    if (a_filePath.empty() ||
        a_filePath.extension() != Constant::k_lowerJsonExtension)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "書き込みが不可能なファイルパスになっており、書き込み処理に失敗しました。");

        return false;
    }

    nlohmann::json l_rootJson = {};

    // Prefab名を保存する
    l_rootJson[Constant::k_gameObjectJsonConverterNameJsonKey] = a_gameObject.GetREFName();

    // GameObjectをシリアライズしてPrefabキーに保存する
    // ConvertToPrefabとClearAllPrefabRemovedUUIDSetは既にGameObjectPrefab::Saveで実行済み
    // ここではGameObjectの状態をそのままJSONへ出力する
    l_rootJson[k_prefabJsonKey] = a_gameObject.Serialize(a_prefabSystem);

    // SceneInstanceUUIDはシーン内でのみ有効な識別子のためPrefabファイルには保存しない
    // 残したままだとPrefab由来の子がインスタンス化される際に
    // 同じUUIDがSceneへ再登録されUUID重複エラーになる
    RemoveSceneInstanceUUIDRecursively(l_rootJson[k_prefabJsonKey]);
 
    // ファイル書き込みに失敗した場合は
    // GameObjectPrefab内部のキャッシュも更新しない
    if (!Utility::SaveJsonFile(l_rootJson, a_filePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "GameObjectPrefabファイルへのJson書き込みに失敗しました。\nFilePath : {}", a_filePath.string());

        return false;
    }

    // 保存成功したらキャッシュを更新する
    a_prefab.SetJson(std::move(l_rootJson[k_prefabJsonKey]));

    return true;
}

void FWK::Converter::GameObjectPrefabJsonConverter::RemoveSceneInstanceUUIDRecursively(nlohmann::json& a_json)
{
    // オブジェクト・配列でなければ子を持たないため終了
    // (プリミティブ値を範囲forで回すと自身を1回だけ列挙するため
    //  終了条件が無いと自身へ無限再帰してしまう)
    const bool l_isObject = a_json.is_object();

    if (!l_isObject &&
        !a_json.is_array())
    {
        return;
    }

    // SceneInstanceUUIDはオブジェクト直下にあるため先に削除
    if (l_isObject)
    {
        a_json.erase(Constant::k_gameObjectJsonConverterSceneInstanceUUIDJsonKey);
    }

    for (auto& l_value : a_json)
    {
        RemoveSceneInstanceUUIDRecursively(l_value);
    }
}