#include "SceneManager.h"

void FWK::SceneManager::LoadScene(const std::filesystem::path& a_nextSceneLoadFilePath)
{	const auto& l_json = Utility::LoadJsonFile(a_nextSceneLoadFilePath);

	if (l_json.is_null()) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "jsonファイルの読み取りに失敗しました。");

		return; 
	}

	// ロード前に初期化を行う
	// (そのシーンで使用するSceneShiftMapなどの情報を消して、次のシーンでしか使用しない情報に置き換えるため)
	INIT();

	m_jsonConverter.Deserialize(l_json, *this);

	m_currentSceneFilePath = a_nextSceneLoadFilePath;

	// Sceneの作成に失敗していればログで出力する
	if (!m_scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Sceneが無効なためSceneのロードに失敗しました。");

		return;
	}

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

void FWK::SceneManager::SaveScene() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, m_currentSceneFilePath);
}
bool FWK::SceneManager::SaveScene(const std::filesystem::path& a_nextSceneLoadFilePath)
{
	if (!m_scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーンが無効なため、シーンの保存に失敗しました。");

		return false;
	}

	if (a_nextSceneLoadFilePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーンの保存先ファイルパスが空のため、シーンの保存に失敗しました。");

		return false;
	}

	if (a_nextSceneLoadFilePath.extension() != Constant::k_lowerJsonExtension)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーンの保存先ファイルがJson形式ではないため、シーンの保存に失敗しました。\nFilePath : {}", a_nextSceneLoadFilePath.string());

		return false;
	}

	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	if (l_rootJson.is_null())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーンのシリアライズに失敗したため、シーンの保存に失敗しました。");

		return false;
	}

	// SaveJsonFile()はFileが存在しない場合でも
	// 新しいJsonファイルを作成できる
	// そのためContent BrowserへのD&Dによる
	// Scene Json新規作成にもこの関数を使用できる。
	if (!Utility::SaveJsonFile(l_rootJson, a_nextSceneLoadFilePath))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor,"シーンJsonの書き込みに失敗しました。\nFilePath : {}", a_nextSceneLoadFilePath.string());

		return false;
	}

	// Jsonファイルへの保存が完全に成功した後でのみ、
	// 現在Sceneの正式なFilePathを変更する。
	//
	// 保存失敗したPathを現在Sceneへ設定しないため。
	m_currentSceneFilePath = a_nextSceneLoadFilePath.lexically_normal();

	return true;
}

bool FWK::SceneManager::AddNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID, const std::filesystem::path& a_nextSceneLoadFilePath)
{
	if (a_sceneUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたUUIDが無効です、追加しようとしたシーン名の確認をしてください。");

		return false;
	}

	if (a_nextSceneLoadFilePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスが空です、追加しようとしたシーンファイルパスの確認をしてください。");

		return false;
	}

	if (!Utility::CanLoadFilePath(a_nextSceneLoadFilePath, Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスがjsonファイルでないか、無効な形式のファイルです、追加しようとしたシーンファイルパスの確認及びファイルの確認をしてください。");

		return false;
	}

	return m_nextSceneLoadFilePathMap.try_emplace(a_sceneUUID, a_nextSceneLoadFilePath).second;
}

void FWK::SceneManager::INIT()
{
	m_nextSceneLoadFilePathMap.clear();

	// 現在保持しているシーンをリセットして新しいシーンを作成
	m_scene = std::make_shared<Scene>();

	m_scene->INIT();
	
	m_nextSceneUUID = {};
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

	const std::filesystem::path l_nextSceneFilePath = *l_nextSceneLoadFilePath;

	// LoadScene(9事態が失敗した場合でも
	// 同じ遷移要求を毎フレーム繰り返さないようにする
	m_nextSceneUUID = {};

	// シーンマネージャーのシーン遷移情報をクリアして
	// シーン遷移情報及びシーンを読み込む
	LoadScene(l_nextSceneFilePath);
}

const std::filesystem::path* FWK::SceneManager::FindPTRNextSceneLoadFilePath(const boost::uuids::uuid& a_uuid) const
{
	const auto& l_itr = m_nextSceneLoadFilePathMap.find(a_uuid);

	if (l_itr == m_nextSceneLoadFilePathMap.end()) { return nullptr; }

	return &l_itr->second;
}