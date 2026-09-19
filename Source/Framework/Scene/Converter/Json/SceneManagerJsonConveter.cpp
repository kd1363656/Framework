#include "SceneManagerJsonConveter.h"

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

    l_scene->Deserialize(l_rootJson
    );
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

    Utility::SaveJsonFile(l_scene->Serialize(), l_currentSceneFilePath);
}