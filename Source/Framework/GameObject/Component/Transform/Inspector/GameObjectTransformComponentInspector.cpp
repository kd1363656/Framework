#include "GameObjectTransformComponentInspector.h"

void FWK::GameObjectTransformComponentInspector::EditInspector(GameObjectTransformComponent& a_transformComponent) const
{
    auto& l_transform        = a_transformComponent.GetMutableREFTransform       ();
    auto& l_matrixUpdateMode = a_transformComponent.GetMutableREFMatrixUpdateMode();

    // 位置
    if (ImGui::DragFloat3(k_transformPositionLabel.data(), &l_transform.m_position.x, Constant::k_imguiDefaultDragValue))
    {
        a_transformComponent.SetShouldUpdateMatrixDirty(true);
    }

    // 回転
    if (auto l_euler = FWK::Utility::QuaternionToEuler(l_transform.m_rotation);
        ImGui::DragFloat3(k_transformRotationLabel.data(), &l_euler.x, Constant::k_imguiDefaultDragValue))
    {
        // オイラー角に変換していたクオータニオンを元に戻して格納
        const auto& l_dragResult = Utility::EulerToQuaternion(l_euler);

        l_transform.m_rotation = l_dragResult;

        a_transformComponent.SetShouldUpdateMatrixDirty(true);
    }

    // 初期スポーン位置はエディターでドラッグしたときのみ決まる拡大率
    if (ImGui::DragFloat3(k_transformScaleLabel.data(), &l_transform.m_scale.x, Constant::k_imguiDefaultDragValue))
    {
        l_transform.m_scale = l_transform.m_scale;

        a_transformComponent.SetShouldUpdateMatrixDirty(true);
    }

    ImGui::SeparatorText(k_matrixStrategyInspectorLabel.data());

    // 行列の計算方法を選択することができるラジオボタンリスト
    // 新しく生成されたらMatrix確定処理を実行
    if (Utility::IMGUIFactoryRadioButtonSelector<TypeAlias::GameObjectTransformComponentMatrixUpdateModeUniqueFactory>(k_matrixStrategySelectorLabel, l_matrixUpdateMode))
    {
        // 関連するポインタとつなげる処理
        l_matrixUpdateMode->PostDeserialize(a_transformComponent);
    }

    // MatrixUpdateModeの編集インスペクターを存在していたら描画
    if (l_matrixUpdateMode)
    {
        l_matrixUpdateMode->EditInspector();
    }
}