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

		void INIT	      ();
		void LoadScene    ();
		void PostLoadScene() const;

		void EarlyUpdate();
		void PreUpdate  ();
		void Update     ();
		void LateUpdate () const;
		void FixMatrix  ();
		
		void SaveScene () const;

	private:

		Scene m_scene = {};
	};
}