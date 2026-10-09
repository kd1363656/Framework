#pragma once

namespace FWK::Struct
{
    struct AssetFilePathData final
    {
        std::filesystem::path m_assetFilePath = {};

        Enum::AssetFilePathType m_type = Enum::AssetFilePathType::Invalid;
    };
}