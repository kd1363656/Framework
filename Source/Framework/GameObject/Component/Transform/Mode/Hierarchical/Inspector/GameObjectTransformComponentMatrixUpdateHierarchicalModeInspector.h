#pragma once

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalMode;
}

namespace FWK
{
    class GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector final
    {
    public:

         GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector() = default;
        ~GameObjectTransformComponentMatrixUpdateHierarchicalModeInspector() = default;

        void EditInspector(GameObjectTransformComponentMatrixUpdateHierarchicalMode& a_matrixUpdateHierarchicalMode);

    private:

        static constexpr std::string_view k_calculateParentWorldMatrixEnumBitShiftCheckBoxSelectorLabel = "親行列計算適用セレクター";
    };
}