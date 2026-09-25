#pragma once

namespace FWK::Enum
{
    enum class SceneChangeEvent
    {
        Invalid,

        Title,
        Game,
        Gameover,   
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        SceneChangeEvent,
        FWK_JSON_ENUM_VALUE(SceneChangeEvent::Invalid),
        FWK_JSON_ENUM_VALUE(SceneChangeEvent::Title),
        FWK_JSON_ENUM_VALUE(SceneChangeEvent::Game),
        FWK_JSON_ENUM_VALUE(SceneChangeEvent::Gameover),
    )
}