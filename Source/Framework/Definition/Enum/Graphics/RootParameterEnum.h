#pragma once

namespace FWK::Enum
{
	enum class RootParameterType
	{
		Invalid,
		CBSpritePass,
		CBSpritePerObject,
		CBCameraPass,
		CBLightPass,
		CBStaticModelPerObject,
		CBSkeletalAnimationLocalMatrix,
		CBSkeletalAnimationVertexSkinning,
		CBFinalColorPass,
		CBFinalPresentPass,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RootParameterType,
		FWK_JSON_ENUM_VALUE(RootParameterType::Invalid),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSpritePerObject),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBCameraPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBLightPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBStaticModelPerObject),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSkeletalAnimationLocalMatrix),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSkeletalAnimationVertexSkinning),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalColorPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalPresentPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::Count),
	)
}