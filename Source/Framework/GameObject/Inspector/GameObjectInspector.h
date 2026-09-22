#pragma once

namespace FWK
{
	class GameObject;
}

namespace FWK
{
    class GameObjectInspector final
    {
    public:

         GameObjectInspector() = default;
        ~GameObjectInspector() = default;

        void EditInspector(GameObject& a_gameObject);
    };
}