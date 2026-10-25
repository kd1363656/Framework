#pragma once

namespace FWK::Concept
{
    // モーフのデータ(m_modelMorphData)を持つModelMeshかどうかを確認するConcept
    // StaticModelとSkeletalAnimationModelで共通のテンプレート(ModelMeshOptimizerなど)が、
    // モーフを持つModelMeshの場合だけ追加の処理を行うために使う
    template <typename Type>
    concept IsModelMorphMeshConcept = requires(Type& a_modelMesh)
    {
        { a_modelMesh.m_modelMorphData } -> std::same_as<Struct::ModelMorphData&>;
    };
}