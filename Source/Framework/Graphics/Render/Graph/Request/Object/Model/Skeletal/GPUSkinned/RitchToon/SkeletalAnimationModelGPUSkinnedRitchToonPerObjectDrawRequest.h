#pragma once

namespace FWK::Graphics
{
    // GPUスキニングで描画するSkeletalAnimationModelの、RitchToonMaterialを使うMeshの描画申請
    // 表面と輪郭線の両方の描画で使う(同じカリング結果の描画コマンドを、PipelineStateだけ変えて2回実行する)
    class SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest final : public SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase
    {
    public:

         SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest()          = default;
        ~SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest() override = default;

        TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const override { return RitchToonMaterial::GetREFTypeINFO().k_staticTypeID; }

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest, SkeletalAnimationModelGPUSkinnedPerObjectDrawRequestBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelGPUSkinnedRitchToonPerObjectDrawRequest)