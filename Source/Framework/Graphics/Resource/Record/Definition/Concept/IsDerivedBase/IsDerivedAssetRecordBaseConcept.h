#pragma once

namespace FWK::Concept
{
	// AssetRecordBaseを継承したRecordかどうかを確認するConcept
	template <typename Type>
	concept IsDerivedAssetRecordBaseConcept = IsDerivedBaseConcept<Type, Graphics::AssetRecordBase>;
}