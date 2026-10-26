#pragma once

namespace FWK::Graphics
{
    // 輪郭線(背面法)で頂点を押し出す向き = 「スムーズ法線」を、モデルの読み込み時に作るクラス
    //
    // なぜ必要か
    //   角ばった所(ハードエッジ)では、同じ位置に「向きの違う法線を持つ頂点」が複数ある
    //   それぞれの法線の向きへ押し出すと、押し出した面が裂けて、輪郭線に隙間ができる
    //   そこで、同じ位置にある頂点同士で法線を平均した向き(スムーズ法線)へ押し出す
    //
    // 作り方
    //   1. 位置がほぼ同じ頂点を、1つのグループにまとめる
    //   2. 三角形ごとの面の向きを、角の大きさで重みを付けて、角の頂点のグループへ足す
    //      (細長い三角形が多い所に引っ張られないように、角度で重みを付ける)
    //   3. 頂点ごとに、自身の法線と大きく逆を向いていない面だけを足して向きを決める
    //      (表と裏が同じ位置にある薄い布は、裏側の面を足すと向きが打ち消されて0になるため)
    //
    // 保存の仕方
    //   スキニングやモーフで頂点が動くと、スムーズ法線も一緒に回る必要がある
    //   そこで「頂点の法線、接線、従法線(TBN)から見た向き」(Tangent空間)で保存する
    //   描画時は、動いた後のTBNで向きを組み立て直すだけなので、スキニングの計算を増やさずに済む
    //   向き(長さ1のベクトル)は、Octahedral符号化で16bit × 2 = uint1つに詰める(GPUのメモリと読み込み量を減らす)
    class ModelOutlineNormalBuilder final
    {
    private:

        // 位置を「k_positionWeldTolerance単位の整数」に丸めた値(この値が同じ頂点を同じ位置とみなす)
        struct PositionKey final
        {
            std::int64_t m_x = k_initialPositionKey;
            std::int64_t m_y = k_initialPositionKey;
            std::int64_t m_z = k_initialPositionKey;

            bool operator==(const PositionKey&) const = default;
        };

        struct PositionKeyHash final
        {
            std::size_t operator()(const PositionKey& a_key) const;
        };

    public:

         ModelOutlineNormalBuilder() = default;
        ~ModelOutlineNormalBuilder() = default;

        // 頂点ごとのスムーズ法線(Tangent空間、Octahedral符号化)の一覧を作る(要素数は頂点数)
        template <typename ModelMeshType>
        std::vector<std::uint32_t> BuildOutlineNormalList(const ModelMeshType& a_modelMesh) const
        {
            const auto& l_modelVertexList = a_modelMesh.m_modelVertexList;
            const auto& l_indexList       = a_modelMesh.m_indexList;

            std::vector<std::uint32_t> l_outlineNormalList(l_modelVertexList.size(), EncodeOctahedralNormal(k_vertexNormalTangentDirection));

            if (l_modelVertexList.empty()) { return l_outlineNormalList; }

            // 1. 位置がほぼ同じ頂点を、1つのグループにまとめる
            std::vector<std::uint32_t> l_vertexGroupIndexList(l_modelVertexList.size(), k_initialGroupIndex);

            std::unordered_map<PositionKey, std::uint32_t, PositionKeyHash> l_groupIndexMap = {};

            for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < l_modelVertexList.size(); ++l_vertexIndex)
            {
                const auto& l_positionKey = CreatePositionKey(l_modelVertexList[l_vertexIndex].m_position);

                // 初めて見た位置なら、新しいグループの番号を割り当てる(既にあれば、そのグループの番号を使う)
                const auto l_groupIndex = static_cast<std::uint32_t>(l_groupIndexMap.size());

                const auto& l_insertResult = l_groupIndexMap.try_emplace(l_positionKey, l_groupIndex);

                l_vertexGroupIndexList[l_vertexIndex] = l_insertResult.first->second;
            }

            // 2. 三角形ごとの面の向き × 角の大きさを、角の頂点のグループへ足す
            std::vector<std::vector<TypeAlias::Math::Vector3>> l_groupFaceNormalList(l_groupIndexMap.size());

            for (std::size_t l_index = 0ULL; l_index + Constant::k_triangleVertexCount <= l_indexList.size(); l_index += Constant::k_triangleVertexCount)
            {
                const std::array<std::uint32_t, Constant::k_triangleVertexCount> l_triangleVertexIndexList =
                {
                    l_indexList[l_index + k_firstCornerOffset],
                    l_indexList[l_index + k_secondCornerOffset],
                    l_indexList[l_index + k_thirdCornerOffset]
                };

                std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount> l_positionList = {};
                std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount> l_normalList   = {};

                for (std::size_t l_corner = 0ULL; l_corner < Constant::k_triangleVertexCount; ++l_corner)
                {
                    const auto& l_modelVertex = l_modelVertexList[l_triangleVertexIndexList[l_corner]];

                    l_positionList[l_corner] = l_modelVertex.m_position;
                    l_normalList  [l_corner] = l_modelVertex.m_normal;
                }

                TypeAlias::Math::Vector3 l_faceNormal = {};

                if (!TryCalculateFaceNormal(l_positionList, l_normalList, l_faceNormal)) { continue; }

                for (std::size_t l_corner = 0ULL; l_corner < Constant::k_triangleVertexCount; ++l_corner)
                {
                    const float l_cornerAngle = CalculateCornerAngle(l_positionList, l_corner);

                    l_groupFaceNormalList[l_vertexGroupIndexList[l_triangleVertexIndexList[l_corner]]].emplace_back(l_faceNormal * l_cornerAngle);
                }
            }

            // 3. 頂点ごとにスムーズ法線を決め、Tangent空間へ変換して詰める
            for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < l_modelVertexList.size(); ++l_vertexIndex)
            {
                const auto& l_modelVertex = l_modelVertexList[l_vertexIndex];

                const auto& l_smoothNormal = CalculateSmoothNormal(l_groupFaceNormalList[l_vertexGroupIndexList[l_vertexIndex]], l_modelVertex.m_normal);

                l_outlineNormalList[l_vertexIndex] = EncodeOctahedralNormal(TransformToTangentSpace(l_smoothNormal, l_modelVertex.m_normal, l_modelVertex.m_tangent));
            }

            return l_outlineNormalList;
        }

    private:

        PositionKey CreatePositionKey(const TypeAlias::Math::Vector3& a_position) const;

        // 三角形の面の向き(長さ1)を求める(頂点の法線と同じ側を向くようにそろえる)
        // 面積がほぼ0の三角形は向きを決められないため、falseを返す
        bool TryCalculateFaceNormal(const std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount>& a_positionList,
                                    const std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount>& a_normalList,
                                          TypeAlias::Math::Vector3&                                               a_faceNormal) const;

        // 三角形のa_corner番目の角の大きさ(ラジアン)
        float CalculateCornerAngle(const std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount>& a_positionList, const std::size_t a_corner) const;

        // グループに足された面の向きのうち、頂点の法線と大きく逆を向いていない物だけを足して、スムーズ法線を求める
        TypeAlias::Math::Vector3 CalculateSmoothNormal(const std::vector<TypeAlias::Math::Vector3>& a_faceNormalList, const TypeAlias::Math::Vector3& a_vertexNormal) const;

        // World(モデルのローカル)空間の向きを、頂点のTBNから見た向き(Tangent空間)へ変換する
        // 接線が使えない頂点は、法線だけから作ったTBN(BuildOrthonormalBasis)を使う
        TypeAlias::Math::Vector3 TransformToTangentSpace(const TypeAlias::Math::Vector3& a_direction,
                                                         const TypeAlias::Math::Vector3& a_normal,
                                                         const TypeAlias::Math::Vector4& a_tangent) const;

        // 法線だけから、法線に直交する接線と従法線を作る
        // ※ HLSL側(ModelRitchToon.hlsliのBuildRitchToonOrthonormalBasis)と必ず同じ作り方にする
        void BuildOrthonormalBasis(const TypeAlias::Math::Vector3& a_normal, TypeAlias::Math::Vector3& a_tangent, TypeAlias::Math::Vector3& a_bitangent) const;

        // 長さ1の向きを、Octahedral符号化で16bit × 2(uint1つ)に詰める
        // ※ HLSL側(ModelRitchToon.hlsliのDecodeRitchToonOctahedralNormal)と必ず同じ詰め方にする
        std::uint32_t EncodeOctahedralNormal(const TypeAlias::Math::Vector3& a_direction) const;

        static constexpr std::int64_t k_initialPositionKey = 0LL;

        static constexpr std::uint32_t k_initialGroupIndex = 0U;

        static constexpr std::size_t k_firstCornerOffset  = 0ULL;
        static constexpr std::size_t k_secondCornerOffset = 1ULL;
        static constexpr std::size_t k_thirdCornerOffset  = 2ULL;

        // この距離(メートル)より近い頂点は同じ位置とみなす
        static constexpr float k_positionWeldTolerance = 0.00001F;

        // 面積・長さがこれより小さい物は、向きを決められないとみなす
        static constexpr float k_minDirectionLengthSquared = 1.0e-12F;

        // 頂点の法線とのdot(cos)がこれより小さい(120度より逆を向いた)面は、スムーズ法線に足さない
        // 角ばった所(90度の角)の面は足し、薄い布の裏側の面(ほぼ180度)は足さないための値
        static constexpr float k_minMergeNormalDot = -0.5F;

        // boost::hash_combineと同じ混ぜ合わせ用の値(黄金比から作った定数と、ビットをずらす量)
        static constexpr std::size_t k_hashMixConstant   = 0x9E3779B97F4A7C15ULL;
        static constexpr std::size_t k_hashMixLeftShift  = 6ULL;
        static constexpr std::size_t k_hashMixRightShift = 2ULL;

        static constexpr float k_zeroValue = 0.0F;
        static constexpr float k_oneValue  = 1.0F;

        static constexpr float k_minCosine = -1.0F;
        static constexpr float k_maxCosine =  1.0F;

        static constexpr TypeAlias::Math::Vector3 k_vertexNormalTangentDirection = { 0.0F, 0.0F, 1.0F };

        // Octahedral符号化で、-1.0F ~ 1.0Fの値を0 ~ 65535へ並べ直すための値
        static constexpr float         k_octahedralRangeScale = 0.5F;
        static constexpr float         k_octahedralRangeBias  = 0.5F;
        static constexpr float         k_octahedralMaxValue   = 65535.0F;
        static constexpr float         k_octahedralRound      = 0.5F;
        static constexpr float         k_octahedralOne        = 1.0F;
        static constexpr float         k_octahedralZero       = 0.0F;
        static constexpr std::uint32_t k_octahedralYShift     = 16U;
    };
}