#pragma once

namespace FWK::Editor
{
	class EditorWindowBase
	{
	public:

				 EditorWindowBase() = default;
		virtual ~EditorWindowBase() = default;

		virtual void PostDeserialize() { /*必要に応じてオーバーライドしてください*/ }

		virtual void Draw() = 0;

		FWK_DEFINE_TYPE_INFO_ROOT(EditorWindowBase)
	};
}