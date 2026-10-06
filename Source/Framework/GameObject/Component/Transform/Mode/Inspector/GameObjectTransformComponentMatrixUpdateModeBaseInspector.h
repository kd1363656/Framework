#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateModeBase;
}

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateModeBaseInspector
    {
    public:

         GameObjectTransformComponentMatrixUpdateModeBaseInspector() = default;
        ~GameObjectTransformComponentMatrixUpdateModeBaseInspector() = default;
    
        void EditInspector(GameObjectTransformComponentMatrixUpdateModeBase& a_gameObjectTransformComponentMatrixUpdateModeBase);

    private:
    
        static constexpr std::string_view k_isRotateAroundPositionJsonKey = "位置を決めてから回転";
    };
}