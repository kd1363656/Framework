#include "SceneManager.h"

// アプリケーション起動時の初回はファイルパスに依存したファイル読み込みになるが
// 次回からはUUIDを通したFilePathの取得になる
void FWK::SceneManager::LoadScene(const std::filesystem::path& a_nextSceneLoadFilePath)
{	
	// ロード前に初期化を行う
	// (そのシーンで使用するSceneShiftMapなどの情報を消して、次のシーンでしか使用しない情報に置き換えるため)
	INIT();

	// 込むためのファイルパスを格納
	m_currentSceneFilePath = a_nextSceneLoadFilePath;

	m_jsonConverter.Load(*this);

	// ファイルパスを格納する
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
	m_jsonConverter.Save(*this);
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
	// Jsonファイルへの保存が完全に成功した後でのみ、
	// 現在Sceneの正式なFilePathを変更する
	// 保存失敗したPathを現在Sceneへ設定しないため。
	m_currentSceneFilePath = a_nextSceneLoadFilePath;

	m_jsonConverter.Save(*this);
	
	return true;
}

bool FWK::SceneManager::AddNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID)
{
	if (a_sceneUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたUUIDが無効です、追加しようとしたシーン名の確認をしてください。");

		return false;
	}

	const auto* l_assetFilePathData = m_assetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID);
	
	if (!l_assetFilePathData)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたアセットファイルパスデータが無効です、追加しようとしたシーンファイルパスの確認をしてください。");

		return false;
	}

	if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "Scene以外のAssetをシーン遷移へ追加しようとしました。");

		return false;
	}

	const auto& l_nextSceneLoadFilePath = l_assetFilePathData->m_assetFilePath;

	if (l_nextSceneLoadFilePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスが空です、追加しようとしたシーンファイルパスの確認をしてください。");

		return false;
	}

	if (!Utility::CanLoadFilePath(l_nextSceneLoadFilePath, Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "シーン遷移に追加しようとしたシーンファイルパスがjsonファイルでないか、無効な形式のファイルです、追加しようとしたシーンファイルパスの確認及びファイルの確認をしてください。");

		return false;
	}

	return m_nextSceneLoadFilePathMap.try_emplace(a_sceneUUID, l_nextSceneLoadFilePath).second;
}
bool FWK::SceneManager::AddNextSceneLoadFilePath(const std::filesystem::path& a_filePath, const boost::uuids::uuid& a_sceneUUID)
{
	if (a_sceneUUID.is_nil())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryへ追加しようとしたSceneUUIDが無効です。");

		return false;
	}

	if (a_filePath.empty())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryへ追加しようとしたSceneFilePathが空です。");

		return false;
	}

	if (!Utility::CanLoadFilePath(
		a_filePath,
		Constant::k_lowerJsonExtension))
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "追加しようとしたSceneFilePathが無効です。\nFilePath : {}", a_filePath.string());

		return false;
	}

	// まずSceneManager側AssetRegistryへSceneとして登録する
	if (!m_assetFilePathRegistry.Add(a_filePath, a_sceneUUID, Enum::AssetFilePathRegistryType::Scene)) { return false; }

	// Registry登録に成功した後、
	// UUIDだけ版を使ってRegistryから正式なPathを取得し、
	// NextSceneLoadFilePathMapへ登録する
	if (AddNextSceneLoadFilePath(a_sceneUUID)) { return true; }

	// NextSceneLoadFilePathMapへの登録に失敗した場合
	// RegistryだけにSceneが残る中途半端な状態を防ぐ
	m_assetFilePathRegistry.Erase(a_filePath);

	return false;
}

bool FWK::SceneManager::RemoveNextSceneLoadFilePath(const boost::uuids::uuid& a_sceneUUID)
{
	if (a_sceneUUID.is_nil()) { return false; }

	bool l_isRemoved = false;

	const auto* l_assetFilePathData = m_assetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID);

	if (l_assetFilePathData &&
		l_assetFilePathData->m_type == Enum::AssetFilePathRegistryType::Scene)
	{
		// Erase()を呼ぶとRegistry内部Dataが消えるため、
		// 先にPathを値として保持する。
		const std::filesystem::path l_sceneFilePath = l_assetFilePathData->m_assetFilePath;

		if (m_assetFilePathRegistry.Erase(
			l_sceneFilePath))
		{
			l_isRemoved = true;
		}
	}

	// unordered_map::erase(Key)は
	// 実際に削除した要素数を返す
	// staleなMap状態だけが残っていた場合でも
	// ここで削除して同期状態へ戻す
	if (m_nextSceneLoadFilePathMap.erase(a_sceneUUID) != static_cast<std::uint64_t>(NULL))
	{
		l_isRemoved = true;
	}

	return l_isRemoved;
}

bool FWK::SceneManager::ReplaceSceneFilePath(const std::filesystem::path& a_oldSceneFilePath, const std::filesystem::path& a_newSceneFilePath, const boost::uuids::uuid& a_sceneUUID)
{
	if (a_oldSceneFilePath.empty() ||
		a_newSceneFilePath.empty() ||
		a_sceneUUID.is_nil())
	{
		return false;
	}

	if (a_oldSceneFilePath == a_newSceneFilePath) { return true; }

	const auto* l_assetFilePathData = m_assetFilePathRegistry.FindPTRAssetFilePathData(a_sceneUUID);

	if (!l_assetFilePathData) { return false; }

	if (l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene ||
		l_assetFilePathData->m_assetFilePath != a_oldSceneFilePath) 
	{
		return false;
	}

	const auto& l_nextSceneFilePathITR = m_nextSceneLoadFilePathMap.find(a_sceneUUID);

	if (l_nextSceneFilePathITR == m_nextSceneLoadFilePathMap.end())
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "SceneManagerのAssetFilePathRegistryにはSceneが存在しますが、NextSceneLoadFilePathMapに存在しません。");

		return false;
	}


	if (l_nextSceneFilePathITR->second !=
		a_oldSceneFilePath)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "AssetFilePathRegistryとNextSceneLoadFilePathMapのSceneFilePathが一致していません。");

		return false;
	}

	// RegistryのUUIDは変更せずPathだけ変更する
	if (!m_assetFilePathRegistry.ReplaceFilePath(a_oldSceneFilePath, a_newSceneFilePath))
	{
		return false;
	}

	// Registry変更に成功した後でScene専用Indexも追従する
	l_nextSceneFilePathITR->second = a_newSceneFilePath;
	
	return true;
}

void FWK::SceneManager::INIT()
{
	m_assetFilePathRegistry.INIT();

	m_nextSceneLoadFilePathMap.clear();

	// 現在保持しているシーンをリセットして新しいシーンを作成
	m_scene = std::make_shared<Scene>();

	m_scene->INIT();
	
	m_currentSceneFilePath.clear();

	m_nextSceneUUID = {};
}

void FWK::SceneManager::LoadNextSceneIfNeeded()
{
	// 次のに移行するシーンの名前が空なら移行しない
	if (m_nextSceneUUID.is_nil()) { return; }

	const auto* l_assetFilePathData = m_assetFilePathRegistry.FindPTRAssetFilePathData(m_nextSceneUUID);

	if (!l_assetFilePathData) 
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "次のシーンへの遷移用のUUIDが無効です、SceneManagerのマップ内部を確認してください。");

		return; 
	}

	// 次のシーンのファイルパスが空なら移行しない
	if (l_assetFilePathData->m_assetFilePath.empty() ||
		l_assetFilePathData->m_type != Enum::AssetFilePathRegistryType::Scene)
	{
		FWK_ADD_LOG(Constant::k_debugWarningColor, "次のシーンへの情報が無効です、SceneManagerのマップ内部を確認してください。");

		return;
	}

	// コピー渡しで次に読み込むファイルのファイルパスがこの関数実行中は絶対に存在するようにする
	const std::filesystem::path l_nextSceneFilePath = l_assetFilePathData->m_assetFilePath;

	// LoadScene(9事態が失敗した場合でも
	// 同じ遷移要求を毎フレーム繰り返さないようにする
	m_nextSceneUUID = {};

	// シーンマネージャーのシーン遷移情報をクリアして
	// シーン遷移情報及びシーンを読み込む
	LoadScene(l_nextSceneFilePath);
}