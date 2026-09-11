#include "AudioManager.h"

FWK::AudioManager::AudioManager() :
	m_audioEngine(nullptr),

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

void FWK::AudioManager::SaveCONFIG() const
{
	const auto& l_rootJson = m_jsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

void FWK::AudioManager::ApplyMasterVolume(const float a_volume)
{
	m_masterVolume = std::clamp(a_volume, k_minMasterVolume, k_maxMasterVolume);

	// AudioEngine生成前にVolumeだけを設定された場合でも、
	// m_masterVolumeには値を保持しておく
	if (!m_audioEngine) { return; }

	m_audioEngine->SetMasterVolume(m_masterVolume);
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