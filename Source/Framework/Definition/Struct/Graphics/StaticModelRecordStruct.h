#pragma once

namespace FWK::Struct
{
    struct StaticModelVertex final
    {
        TypeAlias::Math::Vector3 m_position = {};
        TypeAlias::Math::Vector3 m_normal   = {};
        TypeAlias::Math::Vector4 m_tangent  = {};
        TypeAlias::Math::Vector2 m_uv       = {};
    };

    struct StaticModelMesh final
    {
         StaticModelMesh() = default;
        ~StaticModelMesh() = default;

        StaticModelMesh(const StaticModelMesh&)           = delete;
        StaticModelMesh(      StaticModelMesh&&) noexcept = default;

        StaticModelMesh& operator=(const StaticModelMesh&)           = delete;
        StaticModelMesh& operator=(      StaticModelMesh&&) noexcept = default;

        std::vector<StaticModelVertex> m_modelVertexList = {};
        std::vector<std::uint32_t>     m_indexList       = {};

        Struct::ModelMaterial m_modelMaterial = {};

        // MeshShaderで描画するためのMeshletData
        // FBX読み込み後に、meshoptimizerで作成し、.asset保存/読み込み対象にする
        Struct::ModelMeshletData m_modelMeshletData = {};

        // MeshShader描画時にGPU側で参照するBufferResource群
        // .asset保存対象ではなく、実行時にModelDataから作成する
        Struct::ModelMeshRuntimeDataBase m_modelMeshRuntimeData = {};
    };

    struct StaticModelData final
    {
         StaticModelData() = default;
        ~StaticModelData() = default;

        StaticModelData(const StaticModelData&)           = delete;
        StaticModelData(      StaticModelData&&) noexcept = default;

        StaticModelData& operator=(const StaticModelData&)           = delete;
        StaticModelData& operator=(      StaticModelData&&) noexcept = default;

        std::vector<StaticModelMesh> m_modelMeshList = {};
    };
}