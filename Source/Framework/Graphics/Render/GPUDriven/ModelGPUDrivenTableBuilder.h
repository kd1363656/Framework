#pragma once

namespace FWK::Graphics
{
    // GPUドリブン描画で使う表(インスタンス、Mesh、LOD)を持ち続けるクラス
    //
    // 以前は毎フレーム、全ての描画申請を表へ追加し直していた(500体で1フレーム約1.2ms)
    // 今は表を持ち続け、描画申請の並びが変わったとき(追加、削除、モデルの差し替え)だけ作り直す
    // 並びが変わらないフレームは、ワールド行列などが変わったインスタンスだけを書き換える
    // 表のどこを書き換えたかはVersionで記録し、ModelGPUDrivenDrawerは変わった表だけをGPUへ送る
    //
    // LODはカメラからの距離で毎フレーム変わり、View(カメラ、影のカスケード)ごとにも違うため表には入れず、
    // カリング用ComputeShaderが見えている物についてだけ選ぶ
    //
    // 使い方(1フレームに1回)
    // 1. BeginTable  : 描画申請の数を渡す
    // 2. SetInstance : 描画申請1つごとに、その番号と値を渡す(無効な描画申請はSetInvalidInstance)
    // 3. EndTable    : 表を受け取り、ModelGPUDrivenDrawer::UploadFrameTableへ渡す
    //
    // StaticModel、SkeletalAnimationModelのどちらでも使えるように、
    // モデルの種類(Record)はテンプレートで受け取る
    class ModelGPUDrivenTableBuilder final
    {
    private:

        // 表に入っているモデル1種類分
        struct ModelEntry final
        {
            const void* m_modelRecord = nullptr;

            std::uint64_t m_refreshedFrameCount = k_initialFrameCount;

            // k_skippedBatchIndexなら、描画するMeshが無いモデル(表にはインスタンスを入れない)
            std::size_t m_batchIndex = k_skippedBatchIndex;
        };

        // 描画申請1つが、表のどこに入っているか
        // m_modelEntryIndex   : k_invalidIndexなら、作り直したときに無効だった描画申請
        // m_instanceIndex     : k_invalidIndexなら、表に入っていない(描画するMeshが無い、又は表に入りきらない)
        // m_morphWeightOffset : 無効値なら、ベイクしたウェイトを使う(表に入りきらなかった場合も含む)
        struct InstanceSlot final
        {
            std::size_t m_modelEntryIndex           = k_invalidIndex;
            std::size_t m_localInstanceIndex        = k_invalidIndex;
            std::size_t m_instanceIndex             = k_invalidIndex;
            std::size_t m_requestedMorphWeightCount = k_emptyMorphWeightCount;

            std::uint32_t m_morphWeightOffset = Constant::k_invalidGPUDrivenMorphWeightOffset;
        };

    public:

         ModelGPUDrivenTableBuilder() = default;
        ~ModelGPUDrivenTableBuilder() = default;

        // このフレームの表の更新を始める
        // 作り直しが必要な場合(RequestRebuild済み、又は描画申請の数が変わった)は、表を空にして作り直す
        void BeginTable(const std::size_t a_drawRequestCount);

        // 描画申請1つ分の値を表へ書き込む
        // 作り直し中なら表へ登録し、そうでなければ変わった値だけを書き換える
        // a_skinningInfo : スキニングするモデルの場合のMeshごとの追加情報(スキニングしない場合はnullptr)
        template <typename ModelRecordType>
        void SetInstance(const std::size_t                           a_drawRequestIndex,
                         const ModelRecordType&                      a_modelRecord,
                         const Struct::ModelGPUDrivenInstanceSource& a_instanceSource,
                         const Struct::ModelGPUDrivenSkinningInfo*   a_skinningInfo)
        {
            FWK_ASSERT_RETURN_IF(a_drawRequestIndex >= m_instanceSlotList.size(), "描画申請の番号がBeginTableで渡した数を超えています。");

            if (m_isRebuilding)
            {
                RegisterInstance(a_drawRequestIndex,
                                 a_modelRecord,
                                 a_instanceSource,
                                 a_skinningInfo);

                return;
            }

            UpdateInstance(a_drawRequestIndex,
                           a_modelRecord,
                           a_instanceSource,
                           a_skinningInfo);
        }

        // 描画申請が無効になっている(モデルが破棄されたなど)ことを伝える
        // 前回の作り直しで表に入っていた描画申請なら、並びが変わるため次のフレームで作り直す
        void SetInvalidInstance(const std::size_t a_drawRequestIndex);

        // このフレームの表を完成させて返す
        const Struct::ModelGPUDrivenFrameTable& EndTable();

        // 次のBeginTableで表を作り直す(描画申請を追加したときなどに呼ぶ)
        void RequestRebuild() { m_isRebuildRequired = true; }

        // 表へ追加するMeshのMaterialの種類を設定する(無効値の場合はMaterialの種類を問わず全てのMeshを追加する)
        // 前回と違う種類なら、表に入るMeshが変わるため作り直す
        void ApplyTargetMaterialStaticTypeID(const TypeAlias::StaticTypeID a_targetMaterialStaticTypeID);

    private:

        template <typename ModelRecordType>
        void RegisterInstance(const std::size_t                           a_drawRequestIndex,
                              const ModelRecordType&                      a_modelRecord,
                              const Struct::ModelGPUDrivenInstanceSource& a_instanceSource,
                              const Struct::ModelGPUDrivenSkinningInfo*   a_skinningInfo)
        {
            auto l_modelEntryIndexItr = m_modelEntryIndexMap.find(&a_modelRecord);

            // 初めて出てきたモデルなら、そのモデルのMeshとLODの情報を表へ登録する(全インスタンスで共通のため1回だけ)
            if (l_modelEntryIndexItr == m_modelEntryIndexMap.end())
            {
                ModelEntry l_modelEntry = {};

                l_modelEntry.m_modelRecord         = &a_modelRecord;
                l_modelEntry.m_refreshedFrameCount = m_frameCount;

                Struct::ModelGPUDrivenBatch l_modelBatch = {};

                AppendModelMeshInfo(a_modelRecord, a_skinningInfo, l_modelBatch);

                // 描画するMeshが無い(表に入りきらない、又はこの描画で扱うMaterialのMeshが無い)モデルは、バッチを作らない
                if (l_modelBatch.m_meshCount != Constant::k_initialGPUDrivenBatchValue)
                {
                    l_modelEntry.m_batchIndex = m_frameTable.m_batchList.size();

                    m_frameTable.m_batchList.emplace_back(l_modelBatch);

                    if (m_batchInstanceListList.size() < m_frameTable.m_batchList.size())
                    {
                        m_batchInstanceListList.emplace_back();
                    }
                }

                l_modelEntryIndexItr = m_modelEntryIndexMap.try_emplace(&a_modelRecord, m_modelEntryList.size()).first;

                m_modelEntryList.emplace_back(l_modelEntry);
            }

            auto& l_instanceSlot = m_instanceSlotList[a_drawRequestIndex];

            l_instanceSlot.m_modelEntryIndex = l_modelEntryIndexItr->second;

            const auto& l_modelEntry = m_modelEntryList[l_instanceSlot.m_modelEntryIndex];

            if (l_modelEntry.m_batchIndex == k_skippedBatchIndex) { return; }

            auto l_instance = CreateInstance(a_instanceSource);

            l_instance.m_morphWeightOffset = RegisterMorphWeightList(a_instanceSource, l_instanceSlot);

            // 同じモデルのインスタンスはEndTableで連続して並べるため、ここではモデルごとの一覧へ入れておく
            auto& l_batchInstanceList = m_batchInstanceListList[l_modelEntry.m_batchIndex];

            l_instanceSlot.m_localInstanceIndex = l_batchInstanceList.size();

            l_batchInstanceList.emplace_back(l_instance);
        }

        template <typename ModelRecordType>
        void UpdateInstance(const std::size_t                           a_drawRequestIndex,
                            const ModelRecordType&                      a_modelRecord,
                            const Struct::ModelGPUDrivenInstanceSource& a_instanceSource,
                            const Struct::ModelGPUDrivenSkinningInfo*   a_skinningInfo)
        {
            const auto& l_instanceSlot = m_instanceSlotList[a_drawRequestIndex];

            // 前回の作り直しのときは無効だった描画申請が有効になった場合は、並びが変わるため作り直す
            if (l_instanceSlot.m_modelEntryIndex == k_invalidIndex)
            {
                m_isRebuildRequired = true;

                return;
            }

            auto& l_modelEntry = m_modelEntryList[l_instanceSlot.m_modelEntryIndex];

            // 描画申請のモデルが差し替えられた場合は、並びが変わるため作り直す
            if (l_modelEntry.m_modelRecord != &a_modelRecord)
            {
                m_isRebuildRequired = true;

                return;
            }

            // MaterialのBufferはエディターで編集すると作り直されてSRVの番号が変わるため、
            // Meshの情報はモデルごとに1フレーム1回だけ確認し直す(インスタンスごとには確認しない)
            if (l_modelEntry.m_refreshedFrameCount != m_frameCount)
            {
                l_modelEntry.m_refreshedFrameCount = m_frameCount;

                RefreshModelMeshInfo(a_modelRecord, a_skinningInfo, l_modelEntry);
            }

            if (l_instanceSlot.m_instanceIndex == k_invalidIndex) { return; }

            UpdateMorphWeightList(a_instanceSource, l_instanceSlot);

            auto& l_instance = m_frameTable.m_instanceList[l_instanceSlot.m_instanceIndex];

            // ワールド行列が変わっていないインスタンスは、拡大率などを計算し直さない
            if (std::memcmp(&l_instance.m_worldMatrix, a_instanceSource.m_worldMatrix, sizeof(TypeAlias::Math::Matrix)) != k_sameMemoryCompareResult)
            {
                const auto l_morphWeightOffset = l_instance.m_morphWeightOffset;

                l_instance = CreateInstance(a_instanceSource);

                l_instance.m_morphWeightOffset = l_morphWeightOffset;

                ++m_frameTable.m_instanceTableVersion;
            }

            if (l_instance.m_animationFrameIndex != a_instanceSource.m_animationFrameIndex)
            {
                l_instance.m_animationFrameIndex = a_instanceSource.m_animationFrameIndex;

                ++m_frameTable.m_instanceTableVersion;
            }
        }

        // ModelMesh1つ分のMeshの情報を作る(LODの位置は設定しない)
        // この描画で扱わない種類のMaterialを使うMeshならfalseを返す
        template <typename ModelMeshType>
        bool CreateMeshInfo(const ModelMeshType&                      a_modelMesh,
                            const std::size_t                         a_meshIndex,
                            const Struct::ModelGPUDrivenSkinningInfo* a_skinningInfo,
                                  Struct::SBModelGPUDrivenMeshInfo&   a_meshInfo) const
        {
            const auto& l_modelMeshletData     = a_modelMesh.m_modelMeshletData;
            const auto& l_modelMeshRuntimeData = a_modelMesh.m_modelMeshRuntimeData;
            const auto& l_modelMaterial        = a_modelMesh.m_modelMaterial;
            const auto& l_material             = l_modelMaterial.m_material.GetREFMaterial().lock();

            // この描画で扱わない種類のMaterialを使うMeshは、表へ入れない(別のPassが描画する)
            if (!Utility::IsDrawTargetMaterial(l_material, m_targetMaterialStaticTypeID)) { return false; }

            a_meshInfo.m_boundsCenter = l_modelMeshletData.m_boundsCenter;
            a_meshInfo.m_boundsRadius = l_modelMeshletData.m_boundsRadius;

            // LODで覆われている頂点を沈めるための、ずれの倍率と、頂点ごとの覆われている層の数
            a_meshInfo.m_lodSurfaceDeviationScale                     = l_modelMeshletData.m_lodSurfaceDeviationScale;
            a_meshInfo.m_lodCoveredLayerCountBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_lodCoveredLayerCountBuffer.GetVALSRVDescriptorIndex();

            // 輪郭線を押し出す向き(スムーズ法線)
            a_meshInfo.m_outlineNormalBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_outlineNormalBuffer.GetVALSRVDescriptorIndex();

            // Materialの種類を知らなくても、GPUDataBufferの番号を渡すだけでシェーダー側が自身の型として読む
            a_meshInfo.m_materialBufferSRVDescriptorIndex = l_material->GetVALGPUDataBufferSRVDescriptorIndex();

            a_meshInfo.m_vertexBufferSRVDescriptorIndex            = l_modelMeshRuntimeData.m_vertexBuffer.GetVALSRVDescriptorIndex           ();
            a_meshInfo.m_meshletBufferSRVDescriptorIndex           = l_modelMeshRuntimeData.m_meshletBuffer.GetVALSRVDescriptorIndex          ();
            a_meshInfo.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
            a_meshInfo.m_primitiveIndexBufferSRVDescriptorIndex    = l_modelMeshRuntimeData.m_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
            a_meshInfo.m_meshletBoundsBufferSRVDescriptorIndex     = l_modelMeshRuntimeData.m_meshletBoundsBuffer.GetVALSRVDescriptorIndex    ();

            a_meshInfo.m_allMeshletCount  = static_cast<std::uint32_t>(l_modelMeshletData.m_meshletList.size());
            a_meshInfo.m_allTriangleCount = static_cast<std::uint32_t>(a_modelMesh.m_indexList.size() / Constant::k_triangleVertexCount);

            // スキニングするモデルの場合は、スキニング行列の場所と、動いても全頂点を囲める球の半径を設定する
            if (a_skinningInfo                                        &&
                a_meshIndex < a_skinningInfo->m_paletteBaseList.size() &&
                a_meshIndex < a_skinningInfo->m_boundsRadiusList.size())
            {
                a_meshInfo.m_boundsRadius                           = a_skinningInfo->m_boundsRadiusList[a_meshIndex];
                a_meshInfo.m_skinningMatrixBufferSRVDescriptorIndex = a_skinningInfo->m_skinningMatrixBufferSRVDescriptorIndex;
                a_meshInfo.m_skinningPaletteBase                    = a_skinningInfo->m_paletteBaseList[a_meshIndex];
                a_meshInfo.m_skinningPaletteStride                  = a_skinningInfo->m_paletteStride;
                a_meshInfo.m_skinningFrameCount                     = a_skinningInfo->m_frameCount;
            }

            // モーフを持つMeshの場合は、モーフ用Bufferの場所とベイクしたウェイトの場所を設定する
            SetModelMeshMorphInfo(a_modelMesh, a_skinningInfo, a_meshInfo);

            return true;
        }

        template <typename ModelMeshType>
        void SetModelMeshMorphInfo(const ModelMeshType& a_modelMesh, const Struct::ModelGPUDrivenSkinningInfo* a_skinningInfo, Struct::SBModelGPUDrivenMeshInfo& a_meshInfo) const
        {
            // モーフのデータを持たないModelMesh(StaticModelなど)では何もしない(SRVの番号は無効値のまま)
            if constexpr (Concept::IsModelMorphMeshConcept<ModelMeshType>)
            {
                const auto& l_modelMorphData       = a_modelMesh.m_modelMorphData;
                const auto& l_modelMeshRuntimeData = a_modelMesh.m_modelMeshRuntimeData;

                // モーフを持たないMesh、又はウェイトをベイクしていない(スキニングしない)場合はモーフを行わない
                if (l_modelMorphData.m_targetList.empty() ||
                    !a_skinningInfo)
                {
                    return;
                }

                a_meshInfo.m_morphVertexRangeBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_morphVertexRangeBuffer.GetVALSRVDescriptorIndex();
                a_meshInfo.m_morphDeltaBufferSRVDescriptorIndex       = l_modelMeshRuntimeData.m_morphDeltaBuffer.GetVALSRVDescriptorIndex      ();
                a_meshInfo.m_morphChannelListBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_morphChannelListBuffer.GetVALSRVDescriptorIndex();
                a_meshInfo.m_morphChannelListCount                    = static_cast<std::uint32_t>(l_modelMorphData.m_targetChannelIndexList.size());
                a_meshInfo.m_morphWeightBufferSRVDescriptorIndex      = a_skinningInfo->m_morphWeightBufferSRVDescriptorIndex;
                a_meshInfo.m_morphWeightStride                        = a_skinningInfo->m_morphChannelCount;

                a_meshInfo.m_morphLODTargetErrorBufferSRVDescriptorIndex = l_modelMeshRuntimeData.m_morphLODTargetErrorBuffer.GetVALSRVDescriptorIndex();
            }
        }

        // 作り直しのときに、モデルのMeshの情報とLODを表の最後へ追加する
        template <typename ModelRecordType>
        void AppendModelMeshInfo(const ModelRecordType& a_modelRecord, const Struct::ModelGPUDrivenSkinningInfo* a_skinningInfo, Struct::ModelGPUDrivenBatch& a_modelBatch)
        {
            const auto& l_modelData     = a_modelRecord.GetREFModelData();
            const auto& l_modelMeshList = l_modelData.m_modelMeshList;

            auto& l_meshInfoList = m_frameTable.m_meshInfoList;
            auto& l_lodList      = m_frameTable.m_lodList;

            // 表に入りきらない場合はこのモデルを追加しない
            if (l_meshInfoList.size() + l_modelMeshList.size() > Constant::k_modelGPUDrivenMaxMeshInfoCount) { return; }

            a_modelBatch.m_meshInfoOffset = static_cast<std::uint32_t>(l_meshInfoList.size());

            for (std::size_t l_meshIndex = 0ULL; l_meshIndex < l_modelMeshList.size(); ++l_meshIndex)
            {
                const auto& l_modelMesh = l_modelMeshList[l_meshIndex];

                Struct::SBModelGPUDrivenMeshInfo l_meshInfo = {};

                if (!CreateMeshInfo(l_modelMesh,
                                    l_meshIndex,
                                    a_skinningInfo,
                                    l_meshInfo))
                {
                    continue;
                }

                const auto& l_meshLODList = l_modelMesh.m_modelMeshletData.m_lodList;

                // LODが表に入りきる場合だけLODを使う(入りきらなければ元の形で描画する)
                if (l_lodList.size() + l_meshLODList.size() <= Constant::k_modelGPUDrivenMaxLODCount)
                {
                    l_meshInfo.m_lodOffset = static_cast<std::uint32_t>(l_lodList.size());
                    l_meshInfo.m_lodCount  = static_cast<std::uint32_t>(l_meshLODList.size());

                    l_lodList.insert(l_lodList.end(), l_meshLODList.begin(), l_meshLODList.end());
                }

                l_meshInfoList.emplace_back(l_meshInfo);
            }

            // 表へ追加したMeshの数(この描画で扱わないMaterialのMeshは含まない)
            a_modelBatch.m_meshCount = static_cast<std::uint32_t>(l_meshInfoList.size()) - a_modelBatch.m_meshInfoOffset;
        }

        // 作り直さないフレームで、モデルのMeshの情報を作り直して、変わっていれば表を書き換える
        // 表に入るMeshの数が変わった場合は、並びが変わるため次のフレームで作り直す
        template <typename ModelRecordType>
        void RefreshModelMeshInfo(const ModelRecordType& a_modelRecord, const Struct::ModelGPUDrivenSkinningInfo* a_skinningInfo, const ModelEntry& a_modelEntry)
        {
            const auto& l_modelData     = a_modelRecord.GetREFModelData();
            const auto& l_modelMeshList = l_modelData.m_modelMeshList;

            const bool l_isSkippedModel = a_modelEntry.m_batchIndex == k_skippedBatchIndex;

            std::uint32_t l_meshCount = Constant::k_initialGPUDrivenBatchValue;

            for (std::size_t l_meshIndex = 0ULL; l_meshIndex < l_modelMeshList.size(); ++l_meshIndex)
            {
                Struct::SBModelGPUDrivenMeshInfo l_meshInfo = {};

                if (!CreateMeshInfo(l_modelMeshList[l_meshIndex],
                                    l_meshIndex,
                                    a_skinningInfo,
                                    l_meshInfo))
                {
                    continue;
                }

                // 描画するMeshが無かったモデルに描画するMeshができた、又はMeshが増えた
                if (l_isSkippedModel ||
                    l_meshCount >= m_frameTable.m_batchList[a_modelEntry.m_batchIndex].m_meshCount)
                {
                    m_isRebuildRequired = true;

                    return;
                }

                const auto& l_modelBatch     = m_frameTable.m_batchList[a_modelEntry.m_batchIndex];
                      auto& l_storedMeshInfo = m_frameTable.m_meshInfoList[l_modelBatch.m_meshInfoOffset + l_meshCount];

                // LODの位置は作り直したときに決めた物をそのまま使う(LODの中身はモデルを読み込んだときから変わらない)
                l_meshInfo.m_lodOffset = l_storedMeshInfo.m_lodOffset;
                l_meshInfo.m_lodCount  = l_storedMeshInfo.m_lodCount;

                if (std::memcmp(&l_storedMeshInfo, &l_meshInfo, sizeof(Struct::SBModelGPUDrivenMeshInfo)) != k_sameMemoryCompareResult)
                {
                    l_storedMeshInfo = l_meshInfo;

                    ++m_frameTable.m_meshTableVersion;
                }

                ++l_meshCount;
            }

            // Meshが減った
            if (!l_isSkippedModel &&
                l_meshCount != m_frameTable.m_batchList[a_modelEntry.m_batchIndex].m_meshCount)
            {
                m_isRebuildRequired = true;
            }
        }

        // ワールド行列から、インスタンスの共通部分(行列、拡大率、裏返り、アニメーションのコマ)を作る
        Struct::SBModelGPUDrivenInstance CreateInstance(const Struct::ModelGPUDrivenInstanceSource& a_instanceSource) const;

        // 作り直しのときに、インスタンスだけのモーフのウェイトを表へ追加し、表の中での開始位置を返す
        // ウェイトが無い、又は表に入りきらない場合は無効値を返す(そのインスタンスはベイクしたウェイトを使う)
        std::uint32_t RegisterMorphWeightList(const Struct::ModelGPUDrivenInstanceSource& a_instanceSource, InstanceSlot& a_instanceSlot);

        // 作り直さないフレームで、インスタンスだけのモーフのウェイトが変わっていれば表を書き換える
        // ウェイトの数が変わった(上書きを始めた、やめたなど)場合は、表の中の位置が変わるため次のフレームで作り直す
        void UpdateMorphWeightList(const Struct::ModelGPUDrivenInstanceSource& a_instanceSource, const InstanceSlot& a_instanceSlot);

        // 作り直しの最後に、同じモデルのインスタンスが連続して並ぶように1本の表へまとめる
        void BuildInstanceList();

        static constexpr std::size_t k_skippedBatchIndex     = std::numeric_limits<std::size_t>::max();
        static constexpr std::size_t k_invalidIndex          = std::numeric_limits<std::size_t>::max();
        static constexpr std::size_t k_emptyMorphWeightCount = 0ULL;

        static constexpr std::uint64_t k_initialFrameCount = 0ULL;

        static constexpr int k_sameMemoryCompareResult = 0;

        Struct::ModelGPUDrivenFrameTable m_frameTable = {};

        std::vector<std::vector<Struct::SBModelGPUDrivenInstance>> m_batchInstanceListList = {};

        std::vector<ModelEntry>   m_modelEntryList   = {};
        std::vector<InstanceSlot> m_instanceSlotList = {};

        std::unordered_map<const void*, std::size_t> m_modelEntryIndexMap = {};

        TypeAlias::StaticTypeID m_targetMaterialStaticTypeID = StaticTypeIDGenerator::k_invalidStaticTypeID;

        std::uint64_t m_frameCount = k_initialFrameCount;

        bool m_isRebuildRequired = true;
        bool m_isRebuilding      = false;
    };
}