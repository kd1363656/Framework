#include "StaticModelComponentInspector.h"

void FWK::StaticModelComponentInspector::EditInspector(StaticModelComponent& a_staticModelComponent)
{
	// 描画方法を選択することができるラジオボタンリスト
	if (!Utility::FactoryCheckBoxMapSelector<TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory>(k_drawRequestDataStrategySelectorLabel, a_staticModelComponent.GetMutableRegisterDrawRequestStrategyMap())) { return; }

	a_staticModelComponent.PostDeserialize();
}