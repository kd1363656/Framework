#pragma once

namespace FWK::Editor
{
	class EditorManager final : public Utility::SingletonBase<EditorManager>
	{
	private:

		friend class SingletonBase<EditorManager>;

		 EditorManager();
		~EditorManager() override;


	};
}