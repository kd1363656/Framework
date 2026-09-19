#include "SceneManagerJsonConveter.h"

void FWK::Converter::SceneManagerJsonConverter::DeserializeScene(const nlohmann::json& a_rootJson, Scene& a_scene)
{
    if (a_rootJson.is_null()) { return; }

    a_scene.Deserialize(a_rootJson);
}
nlohmann::json FWK::Converter::SceneManagerJsonConverter::SerializeScene(Scene& a_scene)
{
    return a_scene.Serialize();
}

void FWK::Converter::SceneManagerJsonConverter::Load(SceneManager& a_sceneManager) const
{
    const auto& l_currentSceneFilePath = a_sceneManager.GetREFCurrentSceneFilePath();

    if (!Utility::CanLoadFilePath(l_currentSceneFilePath))
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "読み込めるファイルパスでないため、シーンの読み込みに失敗しました。");

        return;
    }

    const auto& l_rootJson = Utility::LoadJsonFile(l_currentSceneFilePath);

    if (l_rootJson.is_null ()) { return; }

    // シーンのデシリアライズ
    const auto& l_scene = a_sceneManager.GetVALScene().lock();

    if (!l_scene) { return; }

    DeserializeScene(l_rootJson, *l_scene);
}

void FWK::Converter::SceneManagerJsonConverter::Save(const SceneManager& a_sceneManager) const
{
    const auto& l_scene                   = a_sceneManager.GetVALScene                  ().lock();
    const auto& l_currentSceneFilePath    = a_sceneManager.GetREFCurrentSceneFilePath   ();

    if (!l_scene ||
        l_currentSceneFilePath.empty())
    {
        return;
    }

    Utility::SaveJsonFile(SerializeScene(*l_scene), l_currentSceneFilePath);
}