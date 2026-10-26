#pragma once

namespace FWK::Struct
{
    // ギズモで動かしたGameObject1つ分の、動かす前と後のTransform(Undo / Redo用)
    struct TransformGameObjectRecord final
    {
        Transform m_beforeTransform = {};
        Transform m_afterTransform  = {};

        boost::uuids::uuid m_sceneInstanceUUID = {};
    };
}
