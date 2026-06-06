#pragma once

namespace FWK::Tag
{
	struct TextureUsageTagBase final : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageTagBase, TagBase) };

	struct TextureUsageDecideAutoTag final : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageDecideAutoTag, TagBase) };
	struct TextureUsageColorTag      final : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageColorTag,      TagBase) };
	struct TextureUsageNormalTag     final : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageNormalTag,     TagBase) };
	struct TextureUsageDataTag       final : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(TextureUsageDataTag,       TagBase) };
}