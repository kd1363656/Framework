#include "SceneManager.h"

void FWK::SceneManager::LoadScene(const std::filesystem::path& a_nextSceneLoadFilePath)
{
	// ロード前に初期化を行う
	// (そのシーンで使用するSceneShiftMapなどの情報を消して、次のシーンでしか使用しない情報に置き換えるため)
	INIT();

	const auto& l_json = Utility::LoadJsonFile(a_nextSceneLoadFilePath);

	m_jsonConverter.Deserialize(l_json, *this);

	// シーンをロードしてデシリアライズした後の処理
	m_scene->PostDeserialize();
}

void FWK::SceneManager::EarlyUpdate()
{
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
void FWK::SceneManager::PostLateUpdate()
{
	if (!m_scene) { return; }

	m_scene->PostLateUpdate();

	// シーン内部で全ての処理が終わった後に
	// シーンを
	LoadNextSceneIfNeeded();
}

void FWK::SceneManager::SaveScene()
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, m_currentSceneFilePath);
}

void FWK::SceneManager::AddNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID, const std::filesystem::path& a_nextSceneLoadFilePath)
{
	if (a_sceneUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたUUIDが無効です、追加しようとしたシーン名の確認をしてください。");

		return;
	}

	if (a_nextSceneLoadFilePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスが空です、追加しようとしたシーンファイルパスの確認をしてください。");

		return;
	}

	if (!Utility::CanLoadFilePath(a_nextSceneLoadFilePath, Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスがjsonファイルでないか、無効な形式のファイルです、追加しようとしたシーンファイルパスの確認及びファイルの確認をしてください。");

		return;
	}

	m_nextSceneLoadFilePathMap.try_emplace(a_sceneUUID, a_nextSceneLoadFilePath);
}

void FWK::SceneManager::INIT()
{
	m_nextSceneLoadFilePathMap.clear();

	// 現在保持しているシーンをリセットして新しいシーンを作成
	m_scene = std::make_shared<Scene>();

	m_scene->INIT();
	
	m_nextSceneUUID = {};

	m_currentSceneUUID = {};
}

void FWK::SceneManager::LoadNextSceneIfNeeded()
{
	// 次のに移行するシーンの名前が空なら移行しない
	if (m_nextSceneUUID.is_nil()) { return; }

	const auto* l_nextSceneLoadFilePath = FindPTRNextSceneLoadFilePath(m_nextSceneUUID);

	if (!l_nextSceneLoadFilePath) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "次のシーンへの遷移用のUUIDが無効です、SceneManagerのマップ内部を確認してください。");

		return; 
	}

	// 次のシーンのファイルパスが空なら移行しない
	if (l_nextSceneLoadFilePath->empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "次のシーンへのファイルパスが空です、SceneManagerのマップ内部を確認してください。");

		return;
	}

	// シーンマネージャーのシーン遷移情報をクリアして
	// シーン遷移情報及びシーンを読み込む
	LoadScene(*l_nextSceneLoadFilePath);
}

const std::filesystem::path* FWK::SceneManager::FindPTRNextSceneLoadFilePath(const boost::uuids::uuid& a_uuid) const
{
	const auto& l_itr = m_nextSceneLoadFilePathMap.find(a_uuid);

	if (l_itr == m_nextSceneLoadFilePathMap.end()) { return nullptr; }

	return &l_itr->second;
}