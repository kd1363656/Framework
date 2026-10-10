#pragma once

namespace FWK::Enum
{
    enum class RootParameterType
    {
        Invalid,
        CBSpritePass,
        CBSpritePerObject,
        CBCameraPass,
        CBCullingCameraPass,
        CBModelCascadeShadowPass,
        CBLightPass,
        CBCascadeShadowMapPass,
        CBModelPerObject,
        CBSkeletalAnimationVertexSkinningPerObject,
        CBSkeletalAnimationMeshletBoundsUpdatePerObject,
        CBFinalColorPass,
        CBFinalPresentPass,
        RCModelDrawItem,
        RCModelTable,
        RCModelMaterialTable,
        Count,
    };

    FWK_JSON_SERIALIZE_ENUM
    (
        RootParameterType,
        FWK_JSON_ENUM_VALUE(RootParameterType::Invalid),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePerObject),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBCameraPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBCullingCameraPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBModelCascadeShadowPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBLightPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBCascadeShadowMapPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBModelPerObject),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBSkeletalAnimationVertexSkinningPerObject),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBSkeletalAnimationMeshletBoundsUpdatePerObject),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalColorPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalPresentPass),
        FWK_JSON_ENUM_VALUE(RootParameterType::RCModelDrawItem),
        FWK_JSON_ENUM_VALUE(RootParameterType::RCModelTable),
        FWK_JSON_ENUM_VALUE(RootParameterType::RCModelMaterialTable),
        FWK_JSON_ENUM_VALUE(RootParameterType::Count),
    )
}