#pragma once

namespace FWK::Editor
{
	class EditorWindowBase
	{
	public:

				 EditorWindowBase() = default;
		virtual ~EditorWindowBase() = default;

		virtual void Deserialize    (const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };
		virtual void PostDeserialize()                      { /*必要に応じてオーバーライドしてください*/ };

		virtual void Draw() = 0;

		virtual nlohmann::json Serialize() { return nlohmann::json{}; }

		FWK_DEFINE_TYPE_INFO_ROOT(EditorWindowBase)
	};
}