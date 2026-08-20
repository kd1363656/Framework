#include "StaticModelComponentInspector.h"

void FWK::StaticModelComponentInspector::EditInspector(StaticModelComponent& a_staticModelComponent)
{
	std::unique_ptr<StaticModelRegisterDrawRequestStrategyBase> l_staticModelRegistryDrawRequestStrategy = nullptr;

	// 描画方法を選択することができるラジオボタンリスト
	Utility::FactoryRadioButtonSelector<TypeAlias::MatrixStrategyUniqueFactory>(k_drawRequestDataStrategySelectorLabel, l_staticModelRegistryDrawRequestStrategy);
}