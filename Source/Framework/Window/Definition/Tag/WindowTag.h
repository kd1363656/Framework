#pragma once

namespace FWK::Tag
{
	struct WindowStyleTagBase : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleTagBase, TagBase) };

	struct WindowStyleNormalTag               : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleNormalTag,				  TagBase) };
	struct WindowStyleBorderlessTag           : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleBorderlessTag,			  TagBase) };
	struct WindowStyleBorderlessFullScreenTag : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleBorderlessFullScreenTag, TagBase) };
	struct WindowStyleExclusiveFullScreenTag  : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleExclusiveFullScreenTag,  TagBase) };
}