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
		CBModelPerObject,
		CBSkeletalAnimationVertexSkinningPerObject,
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
		FWK_JSON_ENUM_VALUE(RootParameterType::CBModelPerObject),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBSkeletalAnimationVertexSkinningPerObject),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalColorPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::CBFinalPresentPass),
		FWK_JSON_ENUM_VALUE(RootParameterType::Count),
	)
}