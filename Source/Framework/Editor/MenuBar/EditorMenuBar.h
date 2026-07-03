#pragma once

namespace FWK::Editor
{
	class EditorMenu
	{
	public:

		 EditorMenu() = default;
		~EditorMenu() = default;

		void Init();
		void Draw() const;
	};
}