#pragma once

namespace FWK::Graphics
{
    // FBXから取り出した「FBX内の頂点番号ごとの移動量」を、「ModelMeshの頂点番号ごとの移動量(CSR)」へ変換するクラス
    //
    // FBXのブレンドシェイプは、FBX内の頂点(位置1つにつき1つ)ごとに移動量を持つ
    // ModelMeshの頂点は、UVや法線の継ぎ目で同じ位置に複数あり、重複削除や並び替えで番号も変わる
    // そのため、ModelMeshOptimizerで頂点を並び替えた後に、頂点ごとの「FBX内の頂点番号」を使って移動量を割り当てる
    //
    // ※ ModelMeshOptimizerの後、ModelMeshletBuilderの前に呼ぶ(LODの作成でモーフの移動量を使うため)
    class SkeletalAnimationModelMorphBuilder final
    {
    public:

         SkeletalAnimationModelMorphBuilder() = default;
        ~SkeletalAnimationModelMorphBuilder() = default;

        bool BuildModelRecordMorphData(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const;

        // 同じ名前のターゲットをモデル全体で1つのチャンネルにまとめ、ターゲットごとのチャンネル番号を決める
        // ウェイトはチャンネルごとに持つため、複数のMeshにある同じ名前のターゲットは同じウェイトで動く
        // ※ バイナリーファイルには保存しないため、FBXから作った場合もバイナリーファイルから読んだ場合も、読み込んだ後に呼ぶ
        void BuildModelRecordMorphChannel(SkeletalAnimationModelRecord& a_skeletalAnimationModelRecord) const;

    private:

        bool BuildModelMorphData(const std::size_t a_vertexCount, Struct::ModelMorphData& a_modelMorphData) const;

        // FBX内の頂点番号から、その番号を持つModelMeshの頂点番号の一覧を引ける表を作る
        std::vector<std::vector<std::uint32_t>> CreateSourceVertexToVertexListTable(const std::vector<std::uint32_t>& a_sourceVertexIndexList) const;

        // 頂点を1つも動かさないターゲットを取り除き、ターゲットの番号を詰める
        void RemoveUnusedTarget(Struct::ModelMorphData& a_modelMorphData) const;

        void ClearSourceData(Struct::ModelMorphData& a_modelMorphData) const;

        static constexpr std::size_t k_maxIndexToCountOffset = 1ULL;

        static constexpr std::uint32_t k_emptyDeltaCount = 0U;
    };
}