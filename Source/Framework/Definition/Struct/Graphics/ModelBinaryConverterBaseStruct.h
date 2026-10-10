#pragma once

namespace FWK::Struct
{
    struct ModelMeshBinaryHeader final
    {
        std::uint64_t m_vertexCount = Constant::k_emptyModelVertexCount;
        std::uint64_t m_indexCount  = Constant::k_emptyModelIndexCount;

        std::uint64_t m_subMeshNameSize = Constant::k_emptySubMeshNameSize;

        std::uint64_t m_baseColorTextureFilePathSize = Constant::k_emptyTextureFilePathSize;
        std::uint64_t m_normalTextureFilePathSize    = Constant::k_emptyTextureFilePathSize;
        std::uint64_t m_roughnessTextureFilePathSize = Constant::k_emptyTextureFilePathSize;
        std::uint64_t m_metallicTextureFilePathSize  = Constant::k_emptyTextureFilePathSize;

        std::uint64_t m_meshletCount           = Constant::k_emptyModelMeshletCount;
        std::uint64_t m_uniqueVertexIndexCount = Constant::k_emptyModelUniqueVertexIndexCount;
        std::uint64_t m_primitiveIndexCount    = Constant::k_emptyModelPrimitiveIndexCount;
        std::uint64_t m_meshletBoundsCount     = Constant::k_emptyModelMeshletBoundsCount;
    };
}