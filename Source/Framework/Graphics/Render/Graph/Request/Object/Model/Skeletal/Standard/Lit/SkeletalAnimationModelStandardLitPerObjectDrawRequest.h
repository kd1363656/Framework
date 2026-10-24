#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelStandardLitPerObjectDrawRequest final : public SkeletalAnimationModelPerObjectDrawRequestBase
    {
    public:

         SkeletalAnimationModelStandardLitPerObjectDrawRequest()          = default;
        ~SkeletalAnimationModelStandardLitPerObjectDrawRequest() override = default;

        TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const override { return StandardLitMaterial::GetREFTypeINFO().k_staticTypeID; }

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardLitPerObjectDrawRequest, SkeletalAnimationModelPerObjectDrawRequestBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelStandardLitPerObjectDrawRequest)