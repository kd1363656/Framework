#pragma once

namespace FWK::Struct
{
    struct EditorTagSearchPopupText final
    {
        std::string_view m_tagListHeaderText = {};
        std::string_view m_noItemText        = {};
    };
}