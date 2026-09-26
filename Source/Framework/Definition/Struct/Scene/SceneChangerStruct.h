#pragma once

namespace FWK::Struct
{
    struct NextSceneData final
    {
        std::filesystem::path m_filePath = {};

        std::string m_name = {};
    };
}