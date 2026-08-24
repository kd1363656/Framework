#include "TransformComponentInspector.h"

void FWK::TransformComponentInspector::EditInspector(TransformComponent& a_transformComponent)
{
	auto& l_transform               = a_transformComponent.GetMutableREFTransform              ();
	auto& l_initialSettingTransform = a_transformComponent.GetMutableREFInitialSettingTransform();
	auto& l_matrixStrategy          = a_transformComponent.GetMutableREFMatrixStrategy         ();

	// 行列の計算方法を選択することができるラジオボタンリスト
	Utility::FactoryRadioButtonSelector<TypeAlias::MatrixStrategyUniqueFactory>(k_matrixStrategySelectorLabel, l_matrixStrategy);

	// 位置
	if (ImGui::DragFloat3(k_transformPositionLabel.data(), &l_transform.m_position.x, Constant::k_imguiDefaultDragValue))
	{
		l_initialSettingTransform.m_position = l_transform.m_position;
	}

	// 回転
	if (auto l_euler = FWK::Utility::QuaternionToEuler(l_transform.m_rotation);
		ImGui::DragFloat3(k_transformRotationLabel.data(), &l_euler.x, Constant::k_imguiDefaultDragValue))
	{
		// オイラー角に変換していたクオータニオンを元に戻して格納
		auto l_dragResult = Utility::EulerToQuaternion(l_euler);

		l_transform.m_rotation               = l_dragResult;
		l_initialSettingTransform.m_rotation = l_dragResult;
	}

	// 初期スポーン位置はエディターでドラッグしたときのみ決まる拡大率
	if (ImGui::DragFloat3(k_transformScaleLabel.data(), &l_transform.m_scale.x, Constant::k_imguiDefaultDragValue))
	{
		l_initialSettingTransform.m_scale = l_transform.m_scale;
	}
}