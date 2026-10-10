#include "GameObjectBenchmarkComponentInspector.h"

// ※ 注意 : テスト用(GameObjectBenchmarkComponentと一緒に消す)
void FWK::GameObjectBenchmarkComponentInspector::EditInspector(GameObjectBenchmarkComponent& a_benchmarkComponent) const
{
    ImGui::TextDisabled(k_explanationLabel.data());

    // 並べるモデルのファイル(アセットブラウザーからドロップ)
    ImGui::TextUnformatted(k_modelFilePathLabel.data());

    auto& l_modelFilePath = a_benchmarkComponent.GetMutableREFModelFilePath();

    l_modelFilePath.EditInspector();

    // 設定は「並べる」を押したときに使う(押すまで、並べているものは変わらない)
    if (bool l_isSkeletal = a_benchmarkComponent.GetVALIsSkeletal();
        ImGui::Checkbox(k_isSkeletalLabel.data(), &l_isSkeletal))
    {
        a_benchmarkComponent.SetIsSkeletal(l_isSkeletal);
    }

    if (int l_spawnCount = a_benchmarkComponent.GetVALSpawnCount();
        ImGui::DragInt(k_spawnCountLabel.data(), &l_spawnCount, k_spawnCountDragSpeed, k_minSpawnCount, k_maxSpawnCount))
    {
        a_benchmarkComponent.SetSpawnCount(l_spawnCount);
    }

    if (float l_spacing = a_benchmarkComponent.GetVALSpacing();
        ImGui::DragFloat(k_spacingLabel.data(), &l_spacing, k_spacingDragSpeed, k_minSpacing, k_maxSpacing))
    {
        a_benchmarkComponent.SetSpacing(l_spacing);
    }

    // 動かす割合は、毎フレーム行列が変わるモデルの割合(押さなくてもすぐに反映される)
    if (float l_movingRatio = a_benchmarkComponent.GetVALMovingRatio();
        ImGui::SliderFloat(k_movingRatioLabel.data(), &l_movingRatio, k_minMovingRatio, k_maxMovingRatio))
    {
        a_benchmarkComponent.SetMovingRatio(l_movingRatio);
    }

    if (ImGui::Button(k_spawnButtonLabel.data()))
    {
        a_benchmarkComponent.Spawn();
    }

    ImGui::SameLine();

    if (ImGui::Button(k_clearButtonLabel.data()))
    {
        a_benchmarkComponent.Clear();
    }

    const auto& l_spawnedGameObjectList = a_benchmarkComponent.GetREFSpawnedGameObjectList();

    ImGui::Text(k_spawnedCountFormat.data(), static_cast<int>(l_spawnedGameObjectList.size()));
}