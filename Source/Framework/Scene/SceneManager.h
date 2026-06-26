#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	private:

		friend class SingletonBase<SceneManager>;

		 SceneManager()          = default;
		~SceneManager() override = default;

	public:

		void INIT  ();
		void Update();

	private:

		Scene m_scene = {};
	};
}