#pragma once

namespace FWK::Graphics
{
    // FBXのブレンドシェイプ(モーフ)を、ModelMeshのモーフのデータとモーションのウェイトへ変換するクラス
    //
    // FBXでは次の形でブレンドシェイプが保存されている
    //   ufbx_blend_deformer : Mesh1つに付いているブレンドシェイプのまとまり
    //   ufbx_blend_channel  : ターゲット1つ(例 : 「笑顔」)。ウェイトはこの単位でアニメーションする
    //   ufbx_blend_shape    : ターゲットの形。「どの頂点が、どれだけ動くか」の一覧を持つ
    class SkeletalAnimationModelFBXMorphExtractor final
    {
    public:

         SkeletalAnimationModelFBXMorphExtractor() = default;
        ~SkeletalAnimationModelFBXMorphExtractor() = default;

        // FBXのMeshが持つ全ターゲットの名前と、FBX内の頂点番号で表した移動量を取り出す
        // (頂点ごとの「FBX内の頂点番号」は、頂点を作るときにSkeletalAnimationModelFBXLoaderが入れる)
        bool ExtractModelMorphSource(const ufbx_mesh* a_fbxMesh, Struct::ModelMorphData& a_modelMorphData) const;

        // Bake済みのアニメーションから、ターゲットごとのウェイトの変化を取り出す
        // ボーンのキーフレームと同じ時刻で取り出すため、再生時間とフレームレートを受け取る
        bool ExtractModelMorphWeightTrackList(const ufbx_scene*                                 a_fbxScene,
                                              const ufbx_baked_anim*                            a_fbxBakedAnimation,
                                              const double                                      a_animationDurationSecond,
                                              const double                                      a_animationFrameRate,
                                                    std::vector<Struct::ModelMorphWeightTrack>& a_modelMorphWeightTrackList) const;

        // FBXのMeshがブレンドシェイプを持つか
        bool HasMorph(const ufbx_mesh* a_fbxMesh) const;

    private:

        bool ExtractModelMorphSourceDeltaList(const ufbx_mesh* a_fbxMesh, const ufbx_blend_shape* a_fbxBlendShape, std::vector<Struct::ModelMorphSourceDelta>& a_sourceDeltaList) const;

        bool CreateModelMorphWeightTrack(const ufbx_baked_prop&               a_fbxBakedProp,
                                         const std::vector<double>&           a_sampleTimeSecondList,
                                         const std::wstring&                  a_targetName,
                                               Struct::ModelMorphWeightTrack& a_modelMorphWeightTrack) const;

        const ufbx_blend_shape* FindPTRTargetBlendShape(const ufbx_blend_channel* a_fbxBlendChannel) const;

        // FBXのウェイトは0～100(%)で保存されているため、0～1へ変換するときに掛ける
        static constexpr double k_deformPercentToWeightScale = 0.01;

        static constexpr float k_minDeltaLengthSquared = 1.0e-12F;
        static constexpr float k_defaultOffsetWeight   = 1.0F;
        static constexpr float k_emptyWeight           = 0.0F;

        static constexpr std::size_t k_lastKeyFrameOffset = 1ULL;
    };
}