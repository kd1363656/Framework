#pragma once

namespace FWK::Concept
{
    // AssetRecordBaseを継承したRecordかどうかを確認するConcept
    template <typename Type>
    concept IsDerivedGameObjectComponentBaseConcept = IsDerivedBaseConcept<Type, GameObjectComponentBase>;
}