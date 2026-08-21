#include "ModelComponentBaseInspector.h"

void FWK::ModelComponentBaseInspector::EditInspector(ModelComponentBase& a_modelComponentBase)
{
	const auto& l_assetFilePathHelper = a_modelComponentBase.GetVALAssetFilePathHelper().lock();

	if (!l_assetFilePathHelper) { return; }

	l_assetFilePathHelper->EditInspector();
}