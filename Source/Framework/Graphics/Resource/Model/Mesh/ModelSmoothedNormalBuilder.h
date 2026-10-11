#pragma once

namespace FWK::Graphics
{
    template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
    class ModelSmoothedNormalBuilder final
    {
    private:

        struct PositionKeyLess final
        {
            bool operator()(const TypeAlias::Math::Vector3& a_left, const TypeAlias::Math::Vector3& a_right) const
            {
                // std::mapがキーを並べるための「小さいほうか」の判定
                // x → y → zの順に比べ、最初に違いがあった成分の大小で決める(辞書順)
                // 例 : (1,5,0)と(1,2,9)は、xが同じなのでyで比べ、2 < 5のため(1,2,9)のほうが小さい
                // どちらも小さくない2つのキーは、std::mapでは「同じキー」として扱われる
                if (a_left.x != a_right.x) { return a_left.x < a_right.x; }
                if (a_left.y != a_right.y) { return a_left.y < a_right.y; }

                return a_left.z < a_right.z;
            }
        };

        using PositionKeyToGroupIndexMap = std::map<TypeAlias::Math::Vector3, std::size_t, PositionKeyLess>;

    public:

         ModelSmoothedNormalBuilder() = default;
        ~ModelSmoothedNormalBuilder() = default;

        bool BuildModelRecordSmoothedNormal(ModelRecordType& a_modelRecord) const
        {
            // ModelRecordが持つすべてのModelMeshに、アウトライン用の平滑化法線を作る
            // 成功すれば各頂点のm_smoothedNormalに値が入り、失敗すればfalseを返す
            auto& l_modelData = a_modelRecord.GetMutableREFModelData();

            FWK_ASSERT_RETURN_VALUE_IF(l_modelData.m_meshList.empty(), "ModelDataのMeshリストが空のため、平滑化法線の作成に失敗しました。", false);

            for (auto& l_modelMesh : l_modelData.m_meshList)
            {
                // ModelDataはMaterial単位で複数のModelMeshを持つ
                // 平滑化法線はModelMeshの中の頂点同士でだけ平均するため、ModelMeshごとに個別に作る
                FWK_ASSERT_RETURN_VALUE_IF(!BuildModelMeshSmoothedNormal(l_modelMesh), "ModelMeshの平滑化法線の作成に失敗しました。", false);
            }

            return true;
        }

    private:

        bool BuildModelMeshSmoothedNormal(typename ModelRecordType::ModelMesh& a_modelMesh) const
        {
            // 平滑化法線は「同じ位置にある頂点は、すべて同じ向きの法線を持つ」ようにした法線
            // FBXの頂点は、角(ハードエッジ)やUVの継ぎ目で同じ位置に複数の頂点が分かれていて、
            // それぞれが面ごとに違う法線を持っている
            // 例 : 立方体の角(1,1,1)には、法線が(1,0,0) / (0,1,0) / (0,0,1)の3つの頂点がある
            // アウトラインで法線の方向に頂点を押し出すとき、この3つがばらばらに動くと角に隙間ができるため、
            // 3つとも同じ向き(0.577,0.577,0.577)を持たせて、同じ位置へ押し出されるようにする
            auto& l_vertexList = a_modelMesh.m_vertexList;

            FWK_ASSERT_RETURN_VALUE_IF(l_vertexList.empty(),            "ModelMeshの頂点リストが空のため、平滑化法線の作成に失敗しました。",         false);
            FWK_ASSERT_RETURN_VALUE_IF(a_modelMesh.m_indexList.empty(), "ModelMeshのインデックスリストが空のため、平滑化法線の作成に失敗しました。", false);

            // 1. 同じ位置にある頂点を、同じグループにまとめる
            //    l_vertexGroupIndexList[頂点番号]に、その頂点が属するグループの番号が入る
            //    例 : 立方体の角(1,1,1)にある頂点3・7・12は、すべてグループ0になる
            PositionKeyToGroupIndexMap l_positionKeyToGroupIndexMap = {};
            std::vector<std::size_t>   l_vertexGroupIndexList       = {};

            l_vertexGroupIndexList.reserve(l_vertexList.size());

            for (const auto& l_vertex : l_vertexList)
            {
                // まだ無い位置なら、次の番号(今までに作ったグループの数)を新しいグループ番号として登録する
                // try_emplaceは、すでに同じ位置が登録されていれば何もせず、登録済みの要素を指すイテレータを返す
                // そのため、2つ目以降の同じ位置の頂点は、1つ目と同じグループ番号を受け取る
                const auto& l_nextGroupIndex             = l_positionKeyToGroupIndexMap.size       ();
                const auto& l_positionKeyToGroupIndexITR = l_positionKeyToGroupIndexMap.try_emplace(CreatePositionKey(l_vertex.m_position), l_nextGroupIndex).first;

                l_vertexGroupIndexList.emplace_back(l_positionKeyToGroupIndexITR->second);
            }

            // 2. グループごとに、周りの面の向きを足し合わせる
            //    l_groupNormalSumList[グループ番号]に、そのグループの頂点が使われている面の向きの合計が入る
            //    resizeで作った要素は(0,0,0)から始まる
            std::vector<TypeAlias::Math::Vector3> l_groupNormalSumList = {};

            l_groupNormalSumList.resize(l_positionKeyToGroupIndexMap.size());

            // インデックスリストは3つで1つの三角形なので、3つずつ進める
            for (std::size_t l_triangleStartIndex = 0ULL; l_triangleStartIndex < a_modelMesh.m_indexList.size(); l_triangleStartIndex += Constant::k_triangleVertexCount)
            {
                AccumulateTriangleNormal(l_vertexGroupIndexList,
                                         a_modelMesh,
                                         l_triangleStartIndex,
                                         l_groupNormalSumList);
            }

            // 3. 足し合わせた向きを長さ1に揃えて、各頂点の平滑化法線にする
            //    同じグループの頂点は、同じ合計を使うので、同じ向きになる
            for (std::size_t l_vertexIndex = 0ULL; l_vertexIndex < l_vertexList.size(); ++l_vertexIndex)
            {
                auto& l_vertex         = l_vertexList[l_vertexIndex];
                auto  l_smoothedNormal = l_groupNormalSumList[l_vertexGroupIndexList[l_vertexIndex]];

                // 周りの面の向きが打ち消し合って長さがほぼ0になった場合(紙のように薄い裏表の面など)は、
                // 向きが決められないため、元の法線をそのまま使う
                if (l_smoothedNormal.LengthSquared() <= Constant::k_minNormalLengthSquared)
                {
                    l_vertex.m_smoothedNormal = l_vertex.m_normal;

                    continue;
                }

                l_smoothedNormal.Normalize();

                l_vertex.m_smoothedNormal = l_smoothedNormal;
            }

            return true;
        }

        void AccumulateTriangleNormal(const std::vector<std::size_t>&              a_vertexGroupIndexList,
                                      const typename ModelRecordType::ModelMesh&   a_modelMesh,
                                      const std::size_t&                           a_triangleStartIndex,
                                            std::vector<TypeAlias::Math::Vector3>& a_groupNormalSumList) const
        {
            // 1つの三角形の面の向きを求め、三角形の3つの角の頂点が属するグループへ足す
            // 足す量は「面の向き × その角の角度」にする(角度で重み付けする)
            const auto& l_indexList  = a_modelMesh.m_indexList;
            const auto& l_vertexList = a_modelMesh.m_vertexList;

            // 三角形の3つの角の頂点番号と位置、元の法線の合計を集める
            // 例 : a_triangleStartIndex = 6なら、インデックスリストの6・7・8番目が、この三角形の3つの角
            std::array<std::uint32_t, Constant::k_triangleVertexCount>            l_cornerVertexIndexList = {};
            std::array<TypeAlias::Math::Vector3, Constant::k_triangleVertexCount> l_cornerPositionList    = {};
            TypeAlias::Math::Vector3                                              l_vertexNormalSum       = {};

            for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < Constant::k_triangleVertexCount; ++l_cornerIndex)
            {
                const auto  l_vertexIndex = l_indexList[a_triangleStartIndex + l_cornerIndex];
                const auto& l_modelVertex = l_vertexList[l_vertexIndex];

                l_cornerVertexIndexList[l_cornerIndex] = l_vertexIndex;
                l_cornerPositionList[l_cornerIndex]    = l_modelVertex.m_position;

                l_vertexNormalSum += l_modelVertex.m_normal;
            }

            // 1つ目の角から伸びる2本の辺の外積で、三角形の面の向き(面法線)を求める
            // 例 : 角が(0,0,0) / (1,0,0) / (0,1,0)なら、(1,0,0) × (0,1,0) = (0,0,1)で、面はZ方向を向いている
            const auto& l_firstCorner              = l_cornerPositionList[k_firstCornerIndex];
            const auto& l_firstEdge                = l_cornerPositionList[k_secondCornerIndex] - l_firstCorner;
            const auto& l_secondEdge               = l_cornerPositionList[k_thirdCornerIndex]  - l_firstCorner;
                  auto  l_faceNormal               = l_firstEdge.Cross(l_secondEdge);
            const float l_edgeLengthSquaredProduct = l_firstEdge.LengthSquared() * l_secondEdge.LengthSquared();

            // 3点がほぼ一直線に並んだ三角形は、向きが決まらないため足さない
            // 外積の長さの2乗は「辺1の長さの2乗 × 辺2の長さの2乗 × sin²(2辺の間の角度)」なので、
            // 外積の長さの2乗だけで判定すると、三角形が小さいほど小さくなり、一直線でなくても外されてしまう
            // 例 : 辺が1mm(0.001m)の直角三角形でも、外積の長さの2乗は0.000000000001(1兆分の1)しかない
            // そこで「2辺の長さの2乗の積 × k_minNormalLengthSquared」と比べて、sin²だけで判定する
            // sin²が0.000001以下(2辺の間の角度が約0.057度以下)なら一直線とみなす。三角形の大きさには左右されない
            // 辺の長さが0の三角形も、0 <= 0となるため、ここで外れる
            if (l_faceNormal.LengthSquared() <= l_edgeLengthSquaredProduct * Constant::k_minNormalLengthSquared) { return; }

            l_faceNormal.Normalize();

            // 外積の向きは、三角形の角の並び順(時計回り / 反時計回り)によって表裏が逆になる
            // FBXから読んだ元の法線の合計と逆を向いていたら反転して、面の表側を向くように揃える
            if (l_faceNormal.Dot(l_vertexNormalSum) < k_flipThresholdDot)
            {
                l_faceNormal = -l_faceNormal;
            }

            for (std::size_t l_cornerIndex = 0ULL; l_cornerIndex < Constant::k_triangleVertexCount; ++l_cornerIndex)
            {
                // この角から、残り2つの角へ向かう辺の向きを求める
                // 例 : 角0なら、角1へ向かう辺と角2へ向かう辺
                const auto& l_nextCornerIndex     = (l_cornerIndex + k_nextCornerOffset)     % Constant::k_triangleVertexCount;
                const auto& l_previousCornerIndex = (l_cornerIndex + k_previousCornerOffset) % Constant::k_triangleVertexCount;
                      auto  l_toNextEdge          = l_cornerPositionList[l_nextCornerIndex]     - l_cornerPositionList[l_cornerIndex];
                      auto  l_toPreviousEdge      = l_cornerPositionList[l_previousCornerIndex] - l_cornerPositionList[l_cornerIndex];

                l_toNextEdge.Normalize    ();
                l_toPreviousEdge.Normalize();

                // 長さ1の2辺の内積は、その角のcos(角度)になるので、acosで角度(ラジアン)に戻す
                // 例 : 立方体の面の角は90度なので、cos = 0、角度 = 約1.5708
                // 計算誤差で-1〜1をわずかに超えるとacosが計算できないため、範囲に収めてから渡す
                const float l_cornerCosine = std::clamp(l_toNextEdge.Dot(l_toPreviousEdge), k_minCosine, k_maxCosine);
                const float l_cornerAngle  = std::acos (l_cornerCosine);

                // 面の向きを、角度の大きさだけ重み付けして、この角の頂点のグループへ足す
                // 広い角ほど大きく、細長い三角形の鋭い角ほど小さく効くので、
                // 同じ形でも三角形の分け方によって平滑化法線が偏りにくくなる
                const auto& l_groupIndex = a_vertexGroupIndexList[l_cornerVertexIndexList[l_cornerIndex]];

                a_groupNormalSumList[l_groupIndex] += l_faceNormal * l_cornerAngle;
            }
        }

        TypeAlias::Math::Vector3 CreatePositionKey(const TypeAlias::Math::Vector3& a_position) const
        {
            // 位置を0.01mm単位に丸めて、同じ位置かどうかを比べられるキーにする
            // 浮動小数点のまま比べると、FBXの計算誤差で1.0と1.0000001が別の位置として扱われてしまう
            // 100000倍してから四捨五入すると0.01mm未満の差が消え、近い位置は完全に同じ値になる
            // 例 : (1.000001, 0.5, -0.25) → (100000.0, 50000.0, -25000.0)
            // ※ floatは16777216(2の24乗)までの整数をぴったり表せるため、
            //    原点から約167m以内の位置なら、丸めた値が誤差なく一致する
            return TypeAlias::Math::Vector3{ std::round(a_position.x * k_positionQuantizeScale), std::round(a_position.y * k_positionQuantizeScale), std::round(a_position.z * k_positionQuantizeScale) };
        }

        static constexpr float k_positionQuantizeScale =  100000.0F;
        static constexpr float k_flipThresholdDot      =  0.0F;
        static constexpr float k_minCosine             = -1.0F;
        static constexpr float k_maxCosine             =  1.0F;

        static constexpr std::size_t k_firstCornerIndex     = 0ULL;
        static constexpr std::size_t k_secondCornerIndex    = 1ULL;
        static constexpr std::size_t k_thirdCornerIndex     = 2ULL;
        static constexpr std::size_t k_nextCornerOffset     = 1ULL;
        static constexpr std::size_t k_previousCornerOffset = 2ULL;
    };
}