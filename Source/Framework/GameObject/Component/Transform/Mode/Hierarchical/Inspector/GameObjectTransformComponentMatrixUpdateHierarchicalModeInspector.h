#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalMode;
}

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector
    {
    public:

         GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector() = default;
        ~GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector() = default;
    
        void EditInspector(GameObjectTransformComponentMatrixUpdateHierarchicalMode& a_gameObjectTransformComponentMatrixUpdateHierarchicalMode);

    private:
    
        static constexpr std::string_view k_calculateParentWorldMatrixEnumBitShiftCheckBoxSelectorLabel = "親行列計算適用セレクター";
    };
}