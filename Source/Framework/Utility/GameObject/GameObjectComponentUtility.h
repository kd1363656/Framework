#pragma once

namespace FWK::Utility
{
    inline std::weak_ptr<GameObject> FindVALSceneGameObject(const boost::uuids::uuid& a_sceneInstanceUUID)
    {
        const auto& l_sceneManager = SceneManager::GetInstance ();
        const auto& l_scene        = l_sceneManager.GetVALScene().lock();

        if (!l_scene) { return {}; }

        return l_scene->FindVALGameObject(a_sceneInstanceUUID);
    }
}