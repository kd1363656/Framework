#include "AssetFilePathHelperInspector.h"

void FWK::Utility::AssetFilePathHelperInspector::EditInspector(AssetFilePathHelper& a_assetFilePathHelper) const
{
    // 毎回PrefabInspector処理に入ったらダーティーフラグを下げておく
    a_assetFilePathHelper.SetIsFilePathChangedDirty(false);

    a_assetFilePathHelper.SetIsFilePathChangedDirty(true);
}