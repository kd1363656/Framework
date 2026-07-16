#pragma once

namespace FWK::Enum
{
	enum class RenderGraphAccessType
	{
		Invalid,
		Read,
		Write,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphAccessType,
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Read),
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Write),
		FWK_JSON_ENUM_VALUE(RenderGraphAccessType::Count)
	)

	enum class RenderGraphResourceUsage
	{
		Invalid,
		None,
		RenderTarget,
		PixelShaderResource,
		DepthWrite,
		Present,
		Count,
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphResourceUsage,
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::None),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::RenderTarget),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::PixelShaderResource),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::DepthWrite),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Present),
		FWK_JSON_ENUM_VALUE(RenderGraphResourceUsage::Count)
	)

	// ※ 注意
	// このEnumは必ず大まかな種類で分けること(SquarePolygon,Effectなど)
	// PlayerModelかどうかなどで分けないという意味
	enum class RenderGraphPassExecutionLayer
	{
		Invalid,
		Animation,
		Model,
		Sprite,
		PostEffect,
		PhysicsDebug,
		FinalPresent,
		Count
	};

	FWK_JSON_SERIALIZE_ENUM
	(
		RenderGraphPassExecutionLayer,
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::Invalid),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::Animation),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::Model),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::Sprite),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::PostEffect),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::PhysicsDebug),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::FinalPresent),
		FWK_JSON_ENUM_VALUE(RenderGraphPassExecutionLayer::Count)
	)
}