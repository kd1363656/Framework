#pragma once

namespace FWK::Tag
{
	struct WindowStyleTagBase : public TagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleTagBase, TagBase) };

	struct WindowStyleNormalTag               : public WindowStyleTagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleNormalTag,				 WindowStyleTagBase) };
	struct WindowStyleBorderlessFullScreenTag : public WindowStyleTagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleBorderlessFullScreenTag, WindowStyleTagBase) };
	struct WindowStyleFullScreenTag			  : public WindowStyleTagBase { FWK_DEFINE_TYPE_INFO_TAG(WindowStyleFullScreenTag,           WindowStyleTagBase) };
}