#include "SceneManager.h"

void FWK::SceneManager::INIT()
{
	m_scene.INIT();
}
void FWK::SceneManager::LoadScene()
{

}
void FWK::SceneManager::PostLoadScene() const
{

}

void FWK::SceneManager::EarlyUpdate()
{

}
void FWK::SceneManager::PreUpdate()
{

}
void FWK::SceneManager::Update()
{
	m_scene.Update ();
}
void FWK::SceneManager::LateUpdate () const
{
}
void FWK::SceneManager::FixMatrix ()
{
}

void FWK::SceneManager::SaveScene () const
{

}