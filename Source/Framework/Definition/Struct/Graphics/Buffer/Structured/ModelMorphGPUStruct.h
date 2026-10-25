#pragma once

namespace FWK::Struct
{
    // GPUへ送る、頂点1つが受けるモーフのチャンネル1つ分の移動量(16byte)
    // メッシュシェーダーが頂点ごとに何個も読むため、half(16bitの浮動小数点数)に詰めて読む量を半分にしている
    // (移動量は数cm～数十cmなので、halfでも0.1mm程度の細かさで表せる)
    // ※ HLSL側(Shader/Model/Skeletal/GPUSkinning/SkeletalAnimationModelGPUSkinning.hlsliのGPUSkinningMorphVertexDelta)と同じ並びにする
    //    halfを2つ詰めたuintは、下位16bitが1つ目、上位16bitが2つ目
    struct SBModelMorphVertexDelta final
    {
        std::uint32_t m_deltaPositionXY       = Constant::k_initialSBModelMorphVertexDeltaValue;
        std::uint32_t m_deltaPositionZNormalX = Constant::k_initialSBModelMorphVertexDeltaValue;
        std::uint32_t m_deltaNormalYZ         = Constant::k_initialSBModelMorphVertexDeltaValue;
        std::uint32_t m_channelIndex          = Constant::k_invalidModelMorphTargetIndex;
    };

    static_assert(sizeof(SBModelMorphVertexDelta) == 16ULL, "SBModelMorphVertexDeltaのサイズがHLSL側と一致しません。");
}