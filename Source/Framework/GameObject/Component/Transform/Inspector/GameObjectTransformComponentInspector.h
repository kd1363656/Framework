#pragma once

namespace FWK
{
    class GameObjectTransformComponent;
}

namespace FWK
{
    class GameObjectTransformComponentInspector final
    {
    public:

         GameObjectTransformComponentInspector() = default;
        ~GameObjectTransformComponentInspector() = default;

        void EditInspector(GameObjectTransformComponent& a_transformComponent) const;

    private:

        static constexpr std::string_view k_matrixStrategyInspectorLabel = "行列計算方法インスペクター";
        static constexpr std::string_view k_matrixStrategySelectorLabel  = "行列計算方法セレクター";
        static constexpr std::string_view k_transformPositionLabel       = "位置";
        static constexpr std::string_view k_transformRotationLabel       = "回転";
        static constexpr std::string_view k_transformScaleLabel          = "拡大";
    };
}