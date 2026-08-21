#include "StaticModelComponentInspector.h"

void FWK::StaticModelComponentInspector::EditInspector(StaticModelComponent& a_staticModelComponent)
{
	// 描画方法を選択することができるラジオボタンリスト
	Utility::FactoryCheckBoxMapSelector<TypeAlias::StaticModelRegisterDrawRequestStrategyBaseUniqueFactory>(k_drawRequestDataStrategySelectorLabel, a_staticModelComponent.GetMutableRegisterDrawRequestStrategyMap());
}