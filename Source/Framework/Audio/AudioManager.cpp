#include "AudioManager.h"

FWK::AudioManager::AudioManager() :
	m_audioEngine(nullptr),

	m_soundEffectMap(),

	m_jsonConverter(),

	m_masterVolume(Constant::k_defaultVolume),

	m_isAudioDeviceResetRequested(false),

	m_wasAudioDeviceReset(false)
{}
FWK::AudioManager::~AudioManager()
{
	// XAudio2は内部で別スレッドを利用している
	// AudioEngine破棄前に音声処理を停止しておくことで、
	// 終了処理中にAudioEngineが音声データへアクセスすることを防ぐ
	if (!m_audioEngine) { return; }

	m_audioEngine->Suspend();
}

void FWK::AudioManager::INIT()
{
	// 既にAudioEngineが存在する場合は二重初期化しない
	if (m_audioEngine) { return; }

	FWK_ASSERT_RETURN_IF(!CreateAudioEngine(), "AudioEngineの作成に失敗しました");
}

void FWK::AudioManager::LoadCONFIG()
{
	const auto& l_rootJson = Utility::LoadJsonFile(k_configFileIOPath);

	if (l_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(l_rootJson, *this);
}
bool FWK::AudioManager::LoadSoundEffect(const std::filesystem::path& a_filePath)
{
	FWK_ASSERT_RETURN_VALUE_IF(!m_audioEngine,     "AudioEngineが作成されていないため、SoundEffectの読み込みに失敗しました。",   false);
	FWK_ASSERT_RETURN_VALUE_IF(a_filePath.empty(), "SoundEffectのファイルパスが空のため、SoundEffectの読み込みに失敗しました。", false);

	// すでに読み込まれているsoundEffectは再利用する
	if (m_soundEffectMap.contains(a_filePath)) { return true; }
	
	// Filesystem側の例外を使わず、error_codeでファイルの存在を確認する
	std::error_code l_errorCode = {};

	const bool l_isRegularFile = std::filesystem::is_regular_file(a_filePath, l_errorCode);

	FWK_ASSERT_RETURN_VALUE_IF(l_errorCode ||
		                       !l_isRegularFile,
		                       "SoundEffectのファイルが存在しない、または通常ファイルではないため、読み込みに失敗しました。",
		                       false);

	// DirectXTKのSoundEffectは.wavファイルを扱う
	FWK_ASSERT_RETURN_VALUE_IF(a_filePath.extension() != Constant::k_lowerWAVExtension, "SoundEffectへWAV以外のファイルを指定したため、読み込みに失敗しました。", false);

	// SoundEffectはWAVの音声データ自体を所有する
	// AudioEngineの所有権はAudioManagerが持ち、
	// DirectXTKへ渡している生ポインタは一時参照のみ
	auto       l_soundEffect = std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), a_filePath.c_str());
	const bool l_isInserted  = m_soundEffectMap.try_emplace          (a_filePath,          std::move(l_soundEffect)).second;

	FWK_ASSERT_RETURN_VALUE_IF(!l_isInserted, "SoundEffectMapへのSoundEffect登録に失敗しました。", false);

	return true;
}

void FWK::AudioManager::Update()
{
	m_wasAudioDeviceReset = false;

	if (!m_audioEngine) { return; }

	// ヘッドホンやスピーカーの抜き差しなどによって
	// AudioEngineのReset要求が出ている場合、一度だけResetを試す
	if (m_isAudioDeviceResetRequested)
	{
		m_isAudioDeviceResetRequested = false;

		m_wasAudioDeviceReset = m_audioEngine->Reset();

		return;
	}

	// DirectXTKAudioでは毎フレームUpdateが必要
	// Updateがfalseの場合、
	// ・AudioDeviceが存在しない
	// ・AudioDeviceが失われたなどの可能性がある
	if (!m_audioEngine->Update() &&
		m_audioEngine->IsCriticalError())
	{
		// 同一フレームでResetせず
		// 次のUpdateでResetする
		m_isAudioDeviceResetRequested = true;
	}
}

void FWK::AudioManager::Suspend()
{
	if (!m_audioEngine) { return; }

	m_audioEngine->Suspend();
}
void FWK::AudioManager::Resume()
{
	if (!m_audioEngine) { return; }

	m_audioEngine->Resume();
}

void FWK::AudioManager::SaveCONFIG() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

bool FWK::AudioManager::UnloadSoundEffect(const std::filesystem::path& a_filePath)
{
	FWK_ASSERT_RETURN_VALUE_IF(a_filePath.empty(), "SoundEffectのファイルパスが空のため、SoundEffectの解放に失敗しました。", false);

	const auto& l_itr = m_soundEffectMap.find(a_filePath);

	// 未ロードなら解放する対象が存在しない
	if (l_itr == m_soundEffectMap.end()) { return false; }

	if (!l_itr->second)
	{
		m_soundEffectMap.erase(l_itr);

		return true;
	}

	// SoundEffectInstanceはSoundEffectが所有している
	// WAVデータを参照している
	// Instanceが残っている状態でSoundEffectを破棄すると
	// 参照先の音声データが先に消えるため解放しない
	if (l_itr->second->IsInUse()) { return false; }

	m_soundEffectMap.erase(l_itr);

	return true;
}
void FWK::AudioManager::ReleaseUnusedSoundEffects()
{
	for (auto l_itr = m_soundEffectMap.begin();
		l_itr != m_soundEffectMap.end();)
	{
		// SoundEffectInstanceから参照されておらず、
		// OneShot再生も行われていないSoundEffectだけ解放する
		if (!l_itr->second ||
			!l_itr->second->IsInUse())
		{
			l_itr = m_soundEffectMap.erase(l_itr);

			continue;
		}

		++l_itr;
	}
}

bool FWK::AudioManager::PlayOneShot(const std::filesystem::path& a_filePath,
	                                const float                  a_volume, 
	                                const float                  a_pitch,
	                                const float                  a_pan)
{
	if (!LoadSoundEffect(a_filePath)) { return false; }

	const auto& l_itr = m_soundEffectMap.find(a_filePath);

	FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_soundEffectMap.end(), "OneShot再生するSoundEffectが見つかりません。", false);
	FWK_ASSERT_RETURN_VALUE_IF(!l_itr->second,                  "OneShot再生するSoundEffectが無効です。",       false);

	// DirectXTKへ範囲外の値を渡さないようにする
	const float l_volume = std::clamp(a_volume, k_minVolume, k_maxVolume);
	const float l_pitch  = std::clamp(a_pitch,  k_minPitch,  k_maxPitch);
	const float l_pan    = std::clamp(a_pan,    k_minPan,    k_maxPan);

	// OneShotはAudioEngine内部のVoicePoolで管理されるため
	// SoundEffectInstanceを呼び出し側で保持する必要がない
	l_itr->second->Play(l_volume, l_pitch, l_pan);

	return true;
}

std::unique_ptr<DirectX::SoundEffectInstance> FWK::AudioManager::Create2DSoundEffectInstance(const std::filesystem::path& a_filePath)
{
	if (!LoadSoundEffect(a_filePath)) { return nullptr; }

	const auto& l_itr = m_soundEffectMap.find(a_filePath);

	FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_soundEffectMap.end(), "2D用SoundEffectInstanceの作成元SoundEffectが見つかりません。", nullptr);
	FWK_ASSERT_RETURN_VALUE_IF(!l_itr->second,                  "2D用SoundEffectInstanceの作成元SoundEffectが無効です。",       nullptr);

	return l_itr->second->CreateInstance(DirectX::SoundEffectInstance_Default);
}
std::unique_ptr<DirectX::SoundEffectInstance> FWK::AudioManager::Create3DSoundEffectInstance(const std::filesystem::path& a_filePath)
{
	if (!LoadSoundEffect(a_filePath)) { return nullptr; }

	const auto& l_itr = m_soundEffectMap.find(a_filePath);

	FWK_ASSERT_RETURN_VALUE_IF(l_itr == m_soundEffectMap.end(), "2D用SoundEffectInstanceの作成元SoundEffectが見つかりません。", nullptr);
	FWK_ASSERT_RETURN_VALUE_IF(!l_itr->second,                  "2D用SoundEffectInstanceの作成元SoundEffectが無効です。",       nullptr);

	return l_itr->second->CreateInstance(DirectX::SoundEffectInstance_Use3D);
}

void FWK::AudioManager::Apply3D(const DirectX::AudioListener& a_listener, const DirectX::AudioEmitter& a_emitter, DirectX::SoundEffectInstance& a_soundEffectInstance) const
{
	// DirectXTKのApply3Dはデフォルトでは右手座標系
	// 現在のFrameworkはDirectXの左手座標系なので
	// rhcoordsへfalseを渡す
	a_soundEffectInstance.Apply3D(a_listener, a_emitter, k_isRightHandedCoordinates);
}
void FWK::AudioManager::ApplyMasterVolume(const float a_volume)
{
	m_masterVolume = std::clamp(a_volume, k_minVolume, k_maxVolume);

	// AudioEngine生成前にVolumeだけを設定された場合でも、
	// m_masterVolumeには値を保持しておく
	if (!m_audioEngine) { return; }

	m_audioEngine->SetMasterVolume(m_masterVolume);
}

void FWK::AudioManager::RequestAudioDeviceReset()
{
	m_isAudioDeviceResetRequested = true;
}

DirectX::AudioStatistics FWK::AudioManager::FetchVALStatistics() const
{
	if (!m_audioEngine) { return {}; }

	return m_audioEngine->GetStatistics();
}

bool FWK::AudioManager::FetchVALIsAudioDevicePresent() const
{
	if (!m_audioEngine) { return false; }

	return m_audioEngine->IsAudioDevicePresent();
}

bool FWK::AudioManager::CreateAudioEngine()
{
	if (m_audioEngine) { return true; }

	// 3D音源では距離減衰や複数音源の合成によって
	// 最終出力がクリップする可能性があるため、
	// DirectXTK側のMasteringLimiterを有効にする
	auto l_audioEngineFlags = DirectX::AudioEngine_UseMasteringLimiter;

#if defined(_DEBUG)
	l_audioEngineFlags = l_audioEngineFlags | DirectX::AudioEngine_Debug;
#endif

	m_audioEngine = std::make_unique<DirectX::AudioEngine>(l_audioEngineFlags);

	FWK_ASSERT_RETURN_VALUE_IF(!m_audioEngine, "AudioEngineの作成に失敗しました。", false);

	m_audioEngine->SetMasterVolume(m_masterVolume);

	return true;
}