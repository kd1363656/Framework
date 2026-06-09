#pragma once

namespace FWK
{
	class SceneManager final : public Utility::SingletonBase<SceneManager>
	{
	public:

		void INIT  ();
		void Update();

	private:

		Scene m_scene = {};

		//=========================
		// シングルトン
		//=========================
		friend class SingletonBase<SceneManager>;

		 SceneManager()          = default;
		~SceneManager() override = default;
	};
}