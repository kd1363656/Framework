#pragma once

namespace FWK::Graphics
{
    class SkeletalAnimationModelStandardUnLitPerObjectDrawRequest final : public SkeletalAnimationModelPerObjectDrawRequestBase
    {
    public:

         SkeletalAnimationModelStandardUnLitPerObjectDrawRequest()          = default;
        ~SkeletalAnimationModelStandardUnLitPerObjectDrawRequest() override = default;

        TypeAlias::StaticTypeID GetVALTargetMaterialStaticTypeID() const override { return StandardUnLitMaterial::GetREFTypeINFO().k_staticTypeID; }

        FWK_DEFINE_TYPE_INFO(SkeletalAnimationModelStandardUnLitPerObjectDrawRequest, SkeletalAnimationModelPerObjectDrawRequestBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::SkeletalAnimationModelStandardUnLitPerObjectDrawRequest)