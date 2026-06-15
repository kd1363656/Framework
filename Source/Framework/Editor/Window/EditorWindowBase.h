#pragma once

namespace FWK::Editor
{
	class EditorWindowBase
	{
	public:

				 EditorWindowBase() = default;
		virtual ~EditorWindowBase() = default;

		virtual void INIT() { /*必要に応じてオーバーライドしてください*/ }
		virtual void Draw() = 0;

		// EditorManager内に存在するリストに生成していいかどうか
		// Logなどがこのクラスを継承するがstd::unique_ptrで存在して欲しい、リストに生成したくない
		// クラスが存在するから
		virtual bool IsAllowCreateInList() const { return true; }

		FWK_DEFINE_TYPE_INFO_ROOT(EditorWindowBase)
	};
}