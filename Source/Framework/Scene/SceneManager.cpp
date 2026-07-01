#include "SceneManager.h"

void FWK::SceneManager::INIT()
{
	m_scene.INIT();
}
void FWK::SceneManager::LoadScene(const std::string_view& a_sceneFilePath)
{
	// ロード時に使用したパスを保存しておく
	m_currentSceneFilePath = a_sceneFilePath;

	const auto& l_json = Utility::LoadJsonFile(a_sceneFilePath);

	m_jsonConverter.Deserialize(l_json, *this);
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
void FWK::SceneManager::LateUpdate() const
{
}
void FWK::SceneManager::FixMatrix()
{
}

void FWK::SceneManager::SaveScene() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, m_currentSceneFilePath);
}