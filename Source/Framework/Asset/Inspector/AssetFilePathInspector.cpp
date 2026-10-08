#include "AssetFilePathInspector.h"

void FWK::AssetFilePathInspector::EditInspector(AssetFilePath& a_assetFilePath) const
{
    // 毎回PrefabInspector処理に入ったらダーティーフラグを下げておく
    a_assetFilePath.SetIsFilePathChangedDirty(false);

    a_assetFilePath.SetIsFilePathChangedDirty(true);
}