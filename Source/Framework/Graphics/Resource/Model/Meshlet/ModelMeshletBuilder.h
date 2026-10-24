#pragma once

namespace FWK::Graphics
{
    template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
    class ModelMeshletBuilder final
    {
    public:

         ModelMeshletBuilder() = default;
        ~ModelMeshletBuilder() = default;

        bool BuildModelRecordMeshletData(ModelRecordType& a_modelRecord) const
        {
            auto& l_modelMeshList = a_modelRecord.GetMutableREFModelData().m_modelMeshList;

            FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelDataのMeshリストが空のため、ModelMeshletDataの作成に失敗しました。", false);

            for (auto& l_modelMesh : l_modelMeshList)
            {
                // StaticModelRecord内の全StaticModelMeshに対して、
                // MeshShader用のStaticModelMeshletDataを作成する
                FWK_ASSERT_RETURN_VALUE_IF(!BuildModelMeshletData(l_modelMesh), "ModelMeshのModelMeshletData作成に失敗しました。", false);
            }

            return true;
        }

    private:

        bool BuildModelMeshletData(typename ModelRecordType::ModelMesh& a_modelMesh) const
        {
            // 頂点数とインデックス数のチェック
            FWK_ASSERT_RETURN_VALUE_IF(a_modelMesh.m_modelVertexList.size() == Constant::k_emptyModelVertexCount, "ModelMeshの頂点数が0のため、MeshletData作成に失敗しました。",         false);
            FWK_ASSERT_RETURN_VALUE_IF(a_modelMesh.m_indexList.size()       == Constant::k_emptyModelIndexCount,  "ModelMeshのインデックス数が0のため、MeshletData作成に失敗しました。", false);

            // インデックスリストの総数を3で割った時に余りが0でないと、三角形を構成するインデックスリストとして不適切
            FWK_ASSERT_RETURN_VALUE_IF((a_modelMesh.m_indexList.size() % Constant::k_triangleVertexCount) != Constant::k_noRemainder, "ModelMeshのインデックス数が三角形単位ではないため、ModelMeshletData作成に失敗しました。", false);

            // メッシュレットデータの初期化
            // 前回読み込んでいたモデルのメッシュレット情報が残るのを防ぐため
            auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            l_modelMeshletData.m_meshletList.clear          ();
            l_modelMeshletData.m_uniqueVertexIndexList.clear();
            l_modelMeshletData.m_primitiveIndexList.clear   ();
            l_modelMeshletData.m_meshletBoundsList.clear    ();
            l_modelMeshletData.m_lodList.clear              ();

            // LOD選択でカメラからの距離を求めるために、Mesh全体を囲む球を求める
            BuildModelMeshBounds(a_modelMesh);

            // LOD0 : 元の形をそのままMeshlet化する(形の崩れは0)
            FWK_ASSERT_RETURN_VALUE_IF(!AppendModelMeshLOD(a_modelMesh, a_modelMesh.m_indexList, Constant::k_initialMeshLODSimplifyError), "LOD0のMeshletData作成に失敗しました。", false);

            // LOD1以降 : 三角形数を半分ずつ減らした形を作り、Meshletを後ろへ連結していく
            BuildSimplifiedModelMeshLODList(a_modelMesh);

            return true;
        }

        void BuildModelMeshBounds(typename ModelRecordType::ModelMesh& a_modelMesh) const
        {
            auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            // 全頂点を囲む箱(AABB)の中心を球の中心にする
            TypeAlias::Math::Vector3 l_boundsMIN = TypeAlias::Math::Vector3( std::numeric_limits<float>::max());
            TypeAlias::Math::Vector3 l_boundsMAX = TypeAlias::Math::Vector3(-std::numeric_limits<float>::max());

            for (const auto& l_modelVertex : a_modelMesh.m_modelVertexList)
            {
                l_boundsMIN = TypeAlias::Math::Vector3::Min(l_boundsMIN, l_modelVertex.m_position);
                l_boundsMAX = TypeAlias::Math::Vector3::Max(l_boundsMAX, l_modelVertex.m_position);
            }

            l_modelMeshletData.m_boundsCenter = (l_boundsMIN + l_boundsMAX) * k_half;

            // 中心から一番遠い頂点までの距離を半径にする(全頂点が必ず球に入る)
            float l_maxDistanceSquared = Constant::k_initialMeshletDataBoundsRadius;

            for (const auto& l_modelVertex : a_modelMesh.m_modelVertexList)
            {
                l_maxDistanceSquared = std::max(l_maxDistanceSquared, TypeAlias::Math::Vector3::DistanceSquared(l_modelMeshletData.m_boundsCenter, l_modelVertex.m_position));
            }

            l_modelMeshletData.m_boundsRadius = std::sqrt(l_maxDistanceSquared);
        }

        void BuildSimplifiedModelMeshLODList(typename ModelRecordType::ModelMesh& a_modelMesh) const
        {
            const auto& l_sourceIndexList = a_modelMesh.m_indexList;

            std::vector<std::uint32_t> l_simplifiedIndexList = {};

            // 簡略化後のインデックス数は元より多くならないため、元と同じ数を確保しておく
            l_simplifiedIndexList.resize(l_sourceIndexList.size());

            auto  l_previousIndexCount = l_sourceIndexList.size();
            float l_targetRatio        = k_lodTriangleReductionRatio;

            for (std::size_t l_lodIndex = k_firstSimplifiedLODIndex; l_lodIndex < k_maxLODCount; ++l_lodIndex)
            {
                // 目標インデックス数(三角形単位に切り捨てる)
                const auto l_targetTriangleCount = static_cast<std::size_t>(static_cast<float>(l_sourceIndexList.size() / Constant::k_triangleVertexCount) * l_targetRatio);
                const auto l_targetIndexCount    = l_targetTriangleCount * Constant::k_triangleVertexCount;

                // これ以上三角形を減らしても効果が薄いので終了
                if (l_targetTriangleCount < k_minLODTriangleCount) { break; }

                float l_resultError = Constant::k_initialMeshLODSimplifyError;

                // 遠距離用のLODだけ、UVや法線の継ぎ目をまたいだ簡略化(Permissive)を許可する
                const auto l_simplifyOptions = l_lodIndex >= k_firstPermissiveLODIndex ? k_farLODSimplifyOptions : k_lodSimplifyOptions;

                auto l_simplifiedIndexCount = SimplifyModelMesh(a_modelMesh,
                                                                l_targetIndexCount,
                                                                l_simplifyOptions,
                                                                l_simplifiedIndexList,
                                                                l_resultError);

                // 継ぎ目を守る簡略化で減らせなかった場合は、継ぎ目をまたいだ簡略化(Permissive)で試し直す
                if (l_simplifyOptions != k_farLODSimplifyOptions &&
                    !IsEnoughLODReduction(l_simplifiedIndexCount, l_previousIndexCount))
                {
                    l_simplifiedIndexCount = SimplifyModelMesh(a_modelMesh,
                                                               l_targetIndexCount,
                                                               k_farLODSimplifyOptions,
                                                               l_simplifiedIndexList,
                                                               l_resultError);
                }

                // ほとんど減らせなかった場合は、これ以上のLODを作っても意味がないので終了
                // ※ 以前はここで、形のつながりを無視して頂点を格子状にまとめる簡略化(Sloppy)に切り替えていたが、
                //   薄い布などに穴が開き、テクスチャも崩れるうえに、記録される誤差が小さすぎて近距離でも選ばれてしまうため使わない
                if (!IsEnoughLODReduction(l_simplifiedIndexCount, l_previousIndexCount)) { break; }

                std::vector<std::uint32_t> l_lodIndexList(l_simplifiedIndexList.begin(), l_simplifiedIndexList.begin() + static_cast<std::ptrdiff_t>(l_simplifiedIndexCount));

                // GPUの頂点キャッシュに乗りやすい順番に三角形を並べ替える
                // meshopt_optimizeVertexCache(書き込み先、元のインデックス配列、インデックス数、頂点数);
                meshopt_optimizeVertexCache(l_lodIndexList.data(),
                                            l_lodIndexList.data(),
                                            l_lodIndexList.size(),
                                            a_modelMesh.m_modelVertexList.size());

                if (!AppendModelMeshLOD(a_modelMesh, l_lodIndexList, l_resultError)) { break; }

                l_previousIndexCount  = l_simplifiedIndexCount;
                l_targetRatio        *= k_lodTriangleReductionRatio;
            }
        }

        std::size_t SimplifyModelMesh(const typename ModelRecordType::ModelMesh& a_modelMesh,
                                      const std::size_t                          a_targetIndexCount,
                                      const unsigned int                         a_simplifyOptions,
                                            std::vector<std::uint32_t>&          a_simplifiedIndexList,
                                            float&                               a_resultError) const
        {
            const auto& l_sourceIndexList  = a_modelMesh.m_indexList;
            const auto& l_frontModelVertex = a_modelMesh.m_modelVertexList.front();

            const auto* l_vertexPositionData  = &l_frontModelVertex.m_position.x;
            const auto* l_vertexAttributeData = &l_frontModelVertex.m_normal.x;
            const auto  l_modelVertexSize     = sizeof(typename ModelRecordType::ModelVertex);

            // 元の形から目標の三角形数まで、見た目がなるべく崩れないように辺を縮めて三角形を減らす
            // 毎回「元の形」から簡略化するため、返ってくる誤差は常に元の形との差になる
            //
            // 位置のずれだけで誤差を測ると、UVや法線が大きく崩れても誤差が小さいままになり、
            // テクスチャが崩れたLODが近距離でも選ばれてしまう
            // そのため、法線とUVのずれも誤差に含める(meshopt_simplifyWithAttributes)
            //
            // meshopt_simplifyWithAttributes(簡略化後のインデックスの書き込み先、
            //                                元のインデックス配列、
            //                                元のインデックス数、
            //                                頂点座標配列、
            //                                頂点数、
            //                                頂点1個分のbyteサイズ、
            //                                頂点属性配列(法線 -> Tangent -> UVの順に並んだfloat)、
            //                                頂点1個分のbyteサイズ、
            //                                属性ごとの重み、
            //                                属性の数、
            //                                動かしてはいけない頂点の印(今回は無し)、
            //                                目標インデックス数、
            //                                許容する誤差(今回は三角形数で止めたいので上限なし)、
            //                                オプション、
            //                                実際に生じた誤差の書き込み先);
            //
            // オプションについて
            // meshopt_SimplifyErrorAbsolute : 誤差を「モデルの大きさに対する割合」ではなく「ローカル座標の距離」で扱う
            //                                 描画時にそのまま画面上のピクセル数へ換算できるようにするため
            // meshopt_SimplifyPrune         : ボタンなどの小さな離れた部品を、目立たなくなった時点で丸ごと取り除く
            // meshopt_SimplifyPermissive    : UVや法線の継ぎ目(同じ位置に別々の頂点がある所)をまたいで簡略化することを許す
            //                                 継ぎ目を守ると三角形がある程度から減らなくなるため、遠距離用のLODだけで使う
            return meshopt_simplifyWithAttributes(a_simplifiedIndexList.data(),
                                                  l_sourceIndexList.data(),
                                                  l_sourceIndexList.size(),
                                                  l_vertexPositionData,
                                                  a_modelMesh.m_modelVertexList.size(),
                                                  l_modelVertexSize,
                                                  l_vertexAttributeData,
                                                  l_modelVertexSize,
                                                  k_simplifyAttributeWeightList.data(),
                                                  k_simplifyAttributeWeightList.size(),
                                                  nullptr,
                                                  a_targetIndexCount,
                                                  std::numeric_limits<float>::max(),
                                                  a_simplifyOptions,
                                                  &a_resultError);
        }

        bool IsEnoughLODReduction(const std::size_t a_simplifiedIndexCount, const std::size_t a_previousIndexCount) const
        {
            // 前のLODの9割より少なくなっていれば、新しいLODを作る意味がある
            return a_simplifiedIndexCount != Constant::k_emptyModelIndexCount &&
                   static_cast<float>(a_simplifiedIndexCount) <= static_cast<float>(a_previousIndexCount) * k_minLODReductionRequirementRatio;
        }

        bool AppendModelMeshLOD(typename ModelRecordType::ModelMesh& a_modelMesh, const std::vector<std::uint32_t>& a_indexList, const float a_simplifyError) const
        {
            auto& l_modelMeshletData = a_modelMesh.m_modelMeshletData;

            // 連結する前の各リストの要素数
            // 今回作成するMeshletの参照位置は、この数だけ後ろへずらす必要がある
            const auto l_baseMeshletCount           = l_modelMeshletData.m_meshletList.size          ();
            const auto l_baseUniqueVertexIndexCount = l_modelMeshletData.m_uniqueVertexIndexList.size();
            const auto l_basePrimitiveIndexCount    = l_modelMeshletData.m_primitiveIndexList.size   ();

            // Meshlet作成に必要になる最大Meshlet数を取得する
            // meshopt_buildMeshletsBound(インデックス数、
            //                            Meshlet内の最大頂点数、
            //                            Meshlet内の最大三角形数);
            const auto l_maxMeshletCount = meshopt_buildMeshletsBound(a_indexList.size(), Constant::k_maxMeshletVertexCount, Constant::k_maxMeshletPrimitiveCount);

            FWK_ASSERT_RETURN_VALUE_IF(l_maxMeshletCount == k_emptyMeshletCount, "Meshletの最大数が0のため、ModelMeshletData作成に失敗しました。", false);

            std::vector<meshopt_Meshlet> l_meshoptMeshletList        = {};
            std::vector<std::uint32_t>   l_uniqueVertexIndexList     = {};
            std::vector<std::uint8_t>    l_meshoptPrimitiveIndexList = {};
            std::vector<std::uint32_t>   l_packedPrimitiveIndexList  = {};

            // メッシュレット情報を最大Meshlet数分確保する
            l_meshoptMeshletList.resize(l_maxMeshletCount);

            // 一つのMeshletが参照できる頂点数は最大64個、
            // そのため、最大Meshlet数 * 1Meshlet辺りの最大頂点数分の
            // 「元モデル頂点インデックス」を格納できるように確保する
            l_uniqueVertexIndexList.resize(l_maxMeshletCount * Constant::k_maxMeshletVertexCount);

            // 1つのMeshletが持てる三角形数は最大128個。
            // 三角形1つにつき頂点インデックスが3個必要なので、
            // 最大Meshlet数 * 1Meshlet辺りの最大三角形数 * 3個分を確保する
            l_meshoptPrimitiveIndexList.resize(l_maxMeshletCount * Constant::k_maxMeshletPrimitiveCount * Constant::k_triangleVertexCount);

            // meshoptimizerは頂点座標をfloat*とstrideで受け取る
            // reinterpret_castでModelVertex全体をfloat*に見せるより、
            // 先頭頂点のm_position.xを直接渡す方が安全
            const auto* l_vertexPositionData = &a_modelMesh.m_modelVertexList.front().m_position.x;

            // 頂点一つ辺りのサイズ
            const auto& l_modelVertexSize = sizeof(typename ModelRecordType::ModelVertex);

            // 最適化済みの頂点とインデックスからMeshShader用のStaticModelMeshletDataを作成する
            // meshopt_buildMeshlets(出力Meshlet配列、
            //                       出力PrimitiveIndex配列、
            //                       入力Index配列、
            //                       入力Index数、
            //                       入力頂点座標配列、
            //                       入力頂点数、
            //                       入力頂点1個分のbyteサイズ、
            //                       Meshlet内の最大頂点数、
            //                       Meshlet内の最大三角形数、
            //                       ConeCulling用の重み);
            const auto l_meshletCount = meshopt_buildMeshlets(l_meshoptMeshletList.data(),
                                                              l_uniqueVertexIndexList.data(),
                                                              l_meshoptPrimitiveIndexList.data(),
                                                              a_indexList.data(),
                                                              a_indexList.size(),
                                                              l_vertexPositionData,
                                                              a_modelMesh.m_modelVertexList.size(),
                                                              l_modelVertexSize,
                                                              Constant::k_maxMeshletVertexCount,
                                                              Constant::k_maxMeshletPrimitiveCount,
                                                              k_defaultMeshletConeWeight);

            FWK_ASSERT_RETURN_VALUE_IF(l_meshletCount == k_emptyMeshletCount, "MeshletData作成結果のMeshlet数が0です。", false);

            // メッシュレットに必要な分のみ要素を確保
            l_meshoptMeshletList.resize(l_meshletCount);

            const auto& l_lastMeshlet = l_meshoptMeshletList.back();

            // 頂点インデックスリストに必要な分のみ要素を確保(最後のメッシュレットのオフセットと頂点数を足した数が必要なサイズ)
            l_uniqueVertexIndexList.resize(l_lastMeshlet.vertex_offset + l_lastMeshlet.vertex_count);

            const auto l_lastMeshletPrimitiveIndexCount = l_lastMeshlet.triangle_count * Constant::k_triangleVertexCount;

            // 最後のMeshletで実際に使用したPrimitiveIndex数
            const auto& l_usedPrimitiveIndexCount = l_lastMeshlet.triangle_offset + l_lastMeshletPrimitiveIndexCount;

            // プリミティブインデックスリストに必要な分のみ要素を確保
            l_meshoptPrimitiveIndexList.resize(l_usedPrimitiveIndexCount);

            // meshoptimizerが出力したuint8_tのPrimitiveIndexListを、
            // 3個ずつuint32_tへPackして保存する。
            FWK_ASSERT_RETURN_VALUE_IF(!PackPrimitiveIndexList(l_meshoptPrimitiveIndexList, l_usedPrimitiveIndexCount, l_packedPrimitiveIndexList), "PrimitiveIndexListのPack化に失敗しました。", false);

            // メッシュレットの頂点情報、インデックス情報取得用パラメータやカリング用パラメータを格納
            for (std::size_t l_meshletIndex = 0ULL; l_meshletIndex < l_meshletCount; ++l_meshletIndex)
            {
                const auto& l_meshoptMeshlet = l_meshoptMeshletList[l_meshletIndex];

                Struct::ModelMeshlet l_modelMeshlet = {};

                // 頂点情報を格納
                // 前のLODのMeshletの後ろへ連結するため、連結前の要素数だけずらす
                l_modelMeshlet.m_vertexOffset = static_cast<std::uint32_t>(l_baseUniqueVertexIndexCount) + l_meshoptMeshlet.vertex_offset;

                FWK_ASSERT_RETURN_VALUE_IF((l_meshoptMeshlet.triangle_offset % Constant::k_triangleVertexCount) != Constant::k_noRemainder, "Meshletのtriangle_offsetが三角形単位ではありません。", false);

                // 3個Pack方式では、uint32_t1個が三角形1個分のPrimitiveIndexを持つ
                // meshoptimizerのtriangle_offsetはPack前PrimitiveIndex配列上のIndexなので、
                // 3で割ってPack済みPrimitiveIndexBuffer上の開始Indexへ変換し、連結前の要素数だけずらす
                l_modelMeshlet.m_triangleOffset = static_cast<std::uint32_t>(l_basePrimitiveIndexCount) + l_meshoptMeshlet.triangle_offset / Constant::k_triangleVertexCount;

                l_modelMeshlet.m_vertexCount   = l_meshoptMeshlet.vertex_count;
                l_modelMeshlet.m_triangleCount = l_meshoptMeshlet.triangle_count;

                l_modelMeshletData.m_meshletList.emplace_back(l_modelMeshlet);

                // Meshlet単位のFrustum CullingやBackfaceConeCullingに使用する境界情報を作成する
                // meshopt_computeMeshletBounds(入力UniqueVertexIndex、
                //                              入力PrimitiveIndex配列、
                //                              Meshlet内の三角形数、
                //                              入力頂点座標配列、
                //                              入力頂点数、
                //                              入力頂点1個分のbyteサイズ);
                const auto l_meshoptBounds = meshopt_computeMeshletBounds(l_uniqueVertexIndexList.data()     + l_meshoptMeshlet.vertex_offset,
                                                                          l_meshoptPrimitiveIndexList.data() + l_meshoptMeshlet.triangle_offset,
                                                                          l_meshoptMeshlet.triangle_count,
                                                                          l_vertexPositionData,
                                                                          a_modelMesh.m_modelVertexList.size(),
                                                                          l_modelVertexSize);

                // メッシュレットカリング用情報を格納
                Struct::ModelMeshletBounds l_modelMeshletBounds = {};

                // Meshletを囲むBoundingSphereの中心座標。
                // この中心座標と半径を使うことで、Meshlet単位でFrustumCullingを行える
                l_modelMeshletBounds.m_center.x = l_meshoptBounds.center[k_vectorElementIndexX];
                l_modelMeshletBounds.m_center.y = l_meshoptBounds.center[k_vectorElementIndexY];
                l_modelMeshletBounds.m_center.z = l_meshoptBounds.center[k_vectorElementIndexZ];

                // Meshletを囲むBoundingSphereの半径。
                // m_centerを中心とした球の半径
                // FrustumCullingではcenter + radiusを使って、Meshletがカメラに映る可能性があるかを判定する
                l_modelMeshletBounds.m_radius = l_meshoptBounds.radius;

                // BackfaceConeCulling用のConeの頂点位置
                // Meshlet内の三角形群が作る「おおよその向き」をConeとして表すときの起点。
                // ConeCullingではCone情報を使って、Meshlet全体がカメラから見てどうかを判定する
                l_modelMeshletBounds.m_coneApex.x = l_meshoptBounds.cone_apex[k_vectorElementIndexX];
                l_modelMeshletBounds.m_coneApex.y = l_meshoptBounds.cone_apex[k_vectorElementIndexY];
                l_modelMeshletBounds.m_coneApex.z = l_meshoptBounds.cone_apex[k_vectorElementIndexZ];

                // BackfaceConeCulling用のしきい値
                // coneAxisと視線方向の内積を使って、このMeshletがカメラから見て裏向きかどうかを判定するための値
                // この値を使うことで、Meshlet内の三角形が全て裏向きと判断できる場合に、
                // MeshShaderやPixelShaderの処理を省略できる
                l_modelMeshletBounds.m_coneCutoff = l_meshoptBounds.cone_cutoff;

                // Meshlet内の三角形群が、おおよそどちらを向いているかを表す方向ベクトル
                // カメラ方向とこの方向を比較して、Meshlet全体が裏向きなら描画をスキップするために使う
                // Normal単体ではなく、Meshlet内の複数三角形の向きをまとめた代表方向
                l_modelMeshletBounds.m_coneAxis.x = l_meshoptBounds.cone_axis[k_vectorElementIndexX];
                l_modelMeshletBounds.m_coneAxis.y = l_meshoptBounds.cone_axis[k_vectorElementIndexY];
                l_modelMeshletBounds.m_coneAxis.z = l_meshoptBounds.cone_axis[k_vectorElementIndexZ];

                l_modelMeshletData.m_meshletBoundsList.emplace_back(l_modelMeshletBounds);
            }

            // 頂点インデックスとPrimitiveIndexを、前のLODの後ろへ連結する
            l_modelMeshletData.m_uniqueVertexIndexList.insert(l_modelMeshletData.m_uniqueVertexIndexList.end(), l_uniqueVertexIndexList.begin(),    l_uniqueVertexIndexList.end());
            l_modelMeshletData.m_primitiveIndexList.insert   (l_modelMeshletData.m_primitiveIndexList.end(),    l_packedPrimitiveIndexList.begin(), l_packedPrimitiveIndexList.end());

            // このLODが「何番目のMeshletから何個使うか」を記録する
            Struct::ModelMeshLOD l_modelMeshLOD = {};

            l_modelMeshLOD.m_meshletOffset = static_cast<std::uint32_t>(l_baseMeshletCount);
            l_modelMeshLOD.m_meshletCount  = static_cast<std::uint32_t>(l_meshletCount);
            l_modelMeshLOD.m_triangleCount = static_cast<std::uint32_t>(a_indexList.size() / Constant::k_triangleVertexCount);
            l_modelMeshLOD.m_simplifyError = a_simplifyError;

            l_modelMeshletData.m_lodList.emplace_back(l_modelMeshLOD);

            return true;
        }

        bool PackPrimitiveIndexList(const std::vector<std::uint8_t>& a_sourcePrimitiveIndexList, const std::size_t& a_usedPrimitiveIndexCount, std::vector<std::uint32_t>& a_packedPrimitiveIndexList) const
        {
            FWK_ASSERT_RETURN_VALUE_IF(a_sourcePrimitiveIndexList.size() < a_usedPrimitiveIndexCount,                            "Pack対象のPrimitiveIndexListサイズが不足しています。",   false);
            FWK_ASSERT_RETURN_VALUE_IF((a_usedPrimitiveIndexCount % Constant::k_triangleVertexCount) != Constant::k_noRemainder, "PrimitiveIndexListの使用数が三角形単位ではありません。", false);

            // 三角形1個につきPrimitiveIndexは3個
            // 3個Pack方式では、三角形1個をuint32_t1個にPackする
            const auto l_packedPrimitiveIndexCount = a_usedPrimitiveIndexCount / Constant::k_triangleVertexCount;

            a_packedPrimitiveIndexList.clear ();
            a_packedPrimitiveIndexList.resize(l_packedPrimitiveIndexCount);

            for (std::size_t l_triangleIndex = 0ULL; l_triangleIndex < l_packedPrimitiveIndexCount; ++l_triangleIndex)
            {
                // Pack元のuint8_t配列における開始位置。
                // uint32_t1個にuint8_t3個を入れるため、3倍する
                const auto l_sourcePrimitiveIndex = l_triangleIndex * Constant::k_triangleVertexCount;

                const auto l_firstPrimitiveIndex  = a_sourcePrimitiveIndexList[l_sourcePrimitiveIndex + k_firstPrimitiveVertexOffset];
                const auto l_secondPrimitiveIndex = a_sourcePrimitiveIndexList[l_sourcePrimitiveIndex + k_secondPrimitiveVertexOffset];
                const auto l_thirdPrimitiveIndex  = a_sourcePrimitiveIndexList[l_sourcePrimitiveIndex + k_thirdPrimitiveVertexOffset];

                // uint32_t1個に、三角形1個分のPrimitiveIndex3個をPackする。
                // bits 0  : 1個目
                // bits 8  : 2個目
                // bits 16 : 3個目
                // bits 24 : 未使用(詰めることはできるがシェーダーで扱いにくいため詰めない)

                // PrimitiveIndexが4個入ったリストとして扱う
                const auto l_packedValue = (l_firstPrimitiveIndex  << k_firstPackedPrimitiveIndexShiftBit)  |
                                           (l_secondPrimitiveIndex << k_secondPackedPrimitiveIndexShiftBit) |
                                           (l_thirdPrimitiveIndex  << k_thirdPackedPrimitiveIndexShiftBit);

                a_packedPrimitiveIndexList[l_triangleIndex] = l_packedValue;
            }

            return true;
        }

        static constexpr float k_defaultMeshletConeWeight = 0.25F;

        static constexpr float k_lodTriangleReductionRatio = 0.5F;

        static constexpr float k_minLODReductionRequirementRatio = 0.9F;

        static constexpr float k_half = 0.5F;

        // 簡略化の誤差に含める頂点属性の重み(ModelVertexの法線 -> Tangent -> UVの並び)
        // 重みが大きいほど、その属性がずれる簡略化を後回しにする
        // ・法線 : 陰影の崩れを防ぐ
        // ・Tangent : 法線とUVから決まるため、誤差には含めない
        // ・UV : テクスチャの崩れを防ぐ(UVは0～1なので、位置(メートル)よりずれが目立つように重くする)
        static constexpr std::array<float, 9ULL> k_simplifyAttributeWeightList =
        {
            0.5F, 0.5F, 0.5F,
            0.0F, 0.0F, 0.0F, 0.0F,
            1.0F, 1.0F
        };

        static constexpr std::size_t k_maxLODCount = 9ULL;

        static constexpr std::size_t k_firstSimplifiedLODIndex = 1ULL;

        static constexpr std::size_t k_firstPermissiveLODIndex = 3ULL;

        static constexpr std::size_t k_minLODTriangleCount = 64ULL;

        static constexpr std::size_t k_emptyMeshletCount = 0ULL;

        static constexpr std::size_t k_vectorElementIndexX = 0ULL;
        static constexpr std::size_t k_vectorElementIndexY = 1ULL;
        static constexpr std::size_t k_vectorElementIndexZ = 2ULL;

        static constexpr std::uint32_t k_firstPrimitiveVertexOffset  = 0U;
        static constexpr std::uint32_t k_secondPrimitiveVertexOffset = 1U;
        static constexpr std::uint32_t k_thirdPrimitiveVertexOffset  = 2U;

        static constexpr std::uint32_t k_packedPrimitiveIndexBitCount = 8U;

        static constexpr std::uint32_t k_firstPackedPrimitiveIndexShiftBit  = k_firstPrimitiveVertexOffset  * k_packedPrimitiveIndexBitCount;
        static constexpr std::uint32_t k_secondPackedPrimitiveIndexShiftBit = k_secondPrimitiveVertexOffset * k_packedPrimitiveIndexBitCount;
        static constexpr std::uint32_t k_thirdPackedPrimitiveIndexShiftBit  = k_thirdPrimitiveVertexOffset  * k_packedPrimitiveIndexBitCount;

        static constexpr unsigned int k_lodSimplifyOptions    = meshopt_SimplifyErrorAbsolute | meshopt_SimplifyPrune;
        static constexpr unsigned int k_farLODSimplifyOptions = meshopt_SimplifyErrorAbsolute | meshopt_SimplifyPrune | meshopt_SimplifyPermissive;
    };
}