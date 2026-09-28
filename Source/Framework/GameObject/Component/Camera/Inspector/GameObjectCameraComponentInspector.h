#pragma once

namespace FWK
{
    class GameObjectCameraComponent;
}

namespace FWK
{
    class GameObjectCameraComponentInspector final
    {
    public:

         GameObjectCameraComponentInspector() = default;
        ~GameObjectCameraComponentInspector() = default;
    
    
        void EditInspector(GameObjectCameraComponent& a_gameObjectCameraComponent);
    };
}