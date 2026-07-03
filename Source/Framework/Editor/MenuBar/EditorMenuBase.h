#pragma once

namespace FWK::Editor
{
	class EditorMenuBase
	{
	public:

				 EditorMenuBase() = default;
		virtual ~EditorMenuBase() = default;

		virtual void Init() { /*必要に応じてオーバーライドしてください*/ }
		virtual void Draw() = 0;

		FWK_DEFINE_TYPE_INFO_ROOT(EditorMenuBase)
	};
}