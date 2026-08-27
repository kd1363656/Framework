#include "SceneManager.h"

void FWK::SceneManager::LoadScene(const std::string_view& a_sceneFilePath)
{
	// ロード前に初期化を行う
	// (そのシーンで使用するSceneShiftMapなどの情報を消して、次のシーンでしか使用しない情報に置き換えるため)
	INIT();

	// ロード時に使用したパスを保存しておく
	m_currentSceneFilePath = a_sceneFilePath;

	const auto& l_json = Utility::LoadJsonFile(a_sceneFilePath);

	m_jsonConverter.Deserialize(l_json, *this);

	// シーンをロードしてデシリアライズした後の処理
	m_scene->PostDeserialize();
}

void FWK::SceneManager::EarlyUpdate()
{
	SceneShiftIfNeeded();

	if (!m_scene) { return; }

	m_scene->EarlyUpdate();
}
void FWK::SceneManager::Update() const
{
	if (!m_scene) { return; }

	m_scene->Update ();
}
void FWK::SceneManager::LateUpdate() const
{
	if (!m_scene) { return; }

	m_scene->LateUpdate();
}
void FWK::SceneManager::PostLateUpdate() const
{
	if (!m_scene) { return; }

	m_scene->PostLateUpdate();
}

void FWK::SceneManager::SaveScene()
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, m_currentSceneFilePath);
}

void FWK::SceneManager::AddSceneShiftMap(const SceneName& a_sceneName, const SceneFilePath& a_sceneFilePath)
{
	if (a_sceneName.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーン名が空です、追加しようとしたシーン名の確認をしてください。");

		return;
	}

	if (a_sceneFilePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスが空です、追加しようとしたシーンファイルパスの確認をしてください。");

		return;
	}

	if (!Utility::CanLoadFilePath(a_sceneFilePath, Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスがjsonファイルでないか、無効な形式のファイルです、追加しようとしたシーンファイルパスの確認及びファイルの確認をしてください。");

		return;
	}

	m_sceneShiftMap.try_emplace(a_sceneName, a_sceneFilePath);
}

void FWK::SceneManager::INIT()
{
	m_sceneShiftMap.clear();

	// 現在保持しているシーンをリセットして新しいシーンを作成
	m_scene = std::make_shared<Scene>();

	m_scene->INIT();
	
	m_nextSceneName.clear       ();
	m_currentSceneFilePath.clear();
}

void FWK::SceneManager::SceneShiftIfNeeded()
{
	// 次のに移行するシーンの名前が空なら移行しない
	if (m_nextSceneName.empty()) { return; }

	const auto& l_nextSceneFilePath = FindSceneShiftFilePath(m_nextSceneName);

	// 次のシーンのファイルパスが空なら移行しない
	if (l_nextSceneFilePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "次のシーンへのファイルパスが空です、SceneManagerのマップ内部を確認してください。");

		return;
	}

	// シーンマネージャーのシーン遷移情報をクリアして
	// シーン遷移情報及びシーンを読み込む
	LoadScene(l_nextSceneFilePath);
}

std::string FWK::SceneManager::FindSceneShiftFilePath(const std::string_view& a_sceneName) const
{
	const auto& l_itr = m_sceneShiftMap.find(a_sceneName);

	if (l_itr == m_sceneShiftMap.end()) { return {}; }

	return l_itr->second;
}