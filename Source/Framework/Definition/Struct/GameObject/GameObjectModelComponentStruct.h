#pragma once

namespace FWK::Struct
{
    struct GameObjectModelComponentMaterialSlot final
    {
        std::wstring m_subMeshName = {};

        AssetFilePath m_materialFilePath = {};

        Graphics::ModelMaterial m_material = {};
    };

    struct ModelDrawMaterial final
    {
        TypeAlias::StaticTypeID m_tableStaticTypeID = StaticTypeIDGenerator::k_invalidStaticTypeID;

        std::uint32_t m_tableElementIndex = Graphics::GPUElementTable::k_invalidElementIndex;
    };
}