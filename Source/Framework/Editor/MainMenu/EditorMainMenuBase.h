#pragma once

namespace FWK::Editor
{
	class EditorMainMenuBase
	{
	public:

				 EditorMainMenuBase() = default;
		virtual ~EditorMainMenuBase() = default;

		virtual void Init() { /*必要に応じてオーバーライドしてください*/ }

		virtual void Draw() = 0;

		FWK_DEFINE_TYPE_INFO_ROOT(EditorMainMenuBase)
	};
}