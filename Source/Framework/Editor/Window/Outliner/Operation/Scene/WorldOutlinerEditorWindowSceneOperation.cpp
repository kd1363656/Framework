#include "WorldOutlinerEditorWindowSceneOperation.h"
#include "../../../../../../Application/Application.h"

void FWK::Editor::WorldOutlinerEditorWindowSceneOperation::RenameScene(const std::string& a_newName, Scene& a_scene) const
{
    // 空文字列への変更は行わない
    if (a_newName.empty()) { return; }

    a_scene.SetName(a_newName);
}

bool FWK::Editor::WorldOutlinerEditorWindowSceneOperation::RequestLoadSceneFromDrop(const std::vector<std::filesystem::path>& a_droppedFilePathList) const
{
    const auto& l_application           = Application::GetInstance                 ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry();

    // アセットブラウザーからドロップされたファイルの中から、シーンとして登録されている最初の1つを探す
    // (同時に読み込めるシーンは1つだけなので、2つ目以降のシーンは無視する)
    const auto& l_sceneFilePathITR = std::ranges::find_if(a_droppedFilePathList,
                                                          [&l_assetFilePathRegistry](const auto& a_filePath)
                                                          {
                                                              return l_assetFilePathRegistry.ContainsAssetFilePath(a_filePath, Enum::AssetFilePathRegistryType::Scene);
                                                          });

    // シーンファイルが含まれていない(Prefabなどのドロップ)
    if (l_sceneFilePathITR == a_droppedFilePathList.end()) { return false; }

    auto& l_sceneManager = SceneManager::GetInstance();

    // シーンの読み込みはアウトライナーの描画中には行わず、要求だけを出しておく
    // (描画中に作り直すと、描画中のノードが古いシーンのGameObjectを指したままになるため)
    l_sceneManager.RequestLoadScene(*l_sceneFilePathITR);

    // 読み込みに失敗した場合もシーンファイルのドロップとして扱い、PrefabのGameObject生成は行わない
    return true;
}