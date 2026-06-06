#pragma once

namespace FWK::Tag
{
	struct TextureUsageTagBase : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageTagBase, TagBase) };

	struct TextureUsageDecideAutoTag final : public TextureUsageTagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageDecideAutoTag, TextureUsageTagBase) };
	struct TextureUsageColorTag      final : public TextureUsageTagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageColorTag,      TextureUsageTagBase) };
	struct TextureUsageNormalTag     final : public TextureUsageTagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageNormalTag,     TextureUsageTagBase) };
	struct TextureUsageDataTag       final : public TextureUsageTagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageDataTag,       TextureUsageTagBase) };
}