#pragma once

namespace FWK
{
    class GameObjectBenchmarkComponent;
}

namespace FWK
{
    class GameObjectBenchmarkComponentInspector final
    {
    public:

         GameObjectBenchmarkComponentInspector() = default;
        ~GameObjectBenchmarkComponentInspector() = default;

        void EditInspector(GameObjectBenchmarkComponent& a_benchmarkComponent) const;

    private:

        static constexpr std::string_view k_explanationLabel   = "テスト用 : 速さの比較のため、モデルを格子状に並べる(比較が終わったら消す)";
        static constexpr std::string_view k_modelFilePathLabel = "モデル(FBX)";
        static constexpr std::string_view k_isSkeletalLabel    = "スケルタル(骨で動くモデル)";
        static constexpr std::string_view k_spawnCountLabel    = "並べる数";
        static constexpr std::string_view k_spacingLabel       = "間隔";
        static constexpr std::string_view k_movingRatioLabel   = "動かす割合";
        static constexpr std::string_view k_spawnButtonLabel   = "並べる";
        static constexpr std::string_view k_clearButtonLabel   = "全部消す";
        static constexpr std::string_view k_spawnedCountFormat = "並べている数 : %d";

        static constexpr float k_spawnCountDragSpeed = 1.0F;
        static constexpr float k_spacingDragSpeed    = 0.1F;
        static constexpr float k_minSpacing          = 0.0F;
        static constexpr float k_maxSpacing          = 100.0F;
        static constexpr float k_minMovingRatio      = 0.0F;
        static constexpr float k_maxMovingRatio      = 1.0F;

        static constexpr int k_minSpawnCount = 1;
        static constexpr int k_maxSpawnCount = 4096;
    };
}