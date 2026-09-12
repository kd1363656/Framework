#include "AudioManager.h"

FWK::AudioManager::AudioManager() :
    m_soundEffectMap(),

    m_soundEffectInstanceList(),

    m_audioEngine(nullptr),

    m_audioListener(),

    m_jsonConverter(),

    m_masterVolume(Constant::k_defaultVolume),

    m_isAudioDeviceResetRequested(false),
    m_wasAudioDeviceReset        (false)
{}
FWK::AudioManager::~AudioManager()
{
    Release();
}

void FWK::AudioManager::INIT()
{
    // 既にAudioEngineが存在する場合は二重初期化しない
    if (m_audioEngine) { return; }

    // リスナーの方向を真正面にしておく
    m_audioListener.OrientFront = TypeAlias::Math::Vector3::Forward;

    FWK_ASSERT_RETURN_IF(!CreateAudioEngine(), "AudioEngineの作成に失敗しました");
}

void FWK::AudioManager::LoadCONFIG()
{
    if (!m_audioEngine) { return; }

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

    // ストップさせたインスタンスは終了したと判断してリストから削除
    RemoveSoundEffectInstanceIfStopped();
}

void FWK::AudioManager::StopAllSound() const
{
    if (!m_audioEngine) { return; }

    for (const auto& l_soundEffectInstanceWeak : m_soundEffectInstanceList)
    {
        const auto& l_soundEffectInstance = l_soundEffectInstanceWeak.lock();

        if (!l_soundEffectInstance) { continue; }

        l_soundEffectInstance->Stop();
    }
}
void FWK::AudioManager::PauseAllSound() const
{
    if (!m_audioEngine) { return; }

    for (const auto& l_soundEffectInstanceWeak : m_soundEffectInstanceList)
    {
        const auto& l_soundEffectInstance = l_soundEffectInstanceWeak.lock();

        if (!l_soundEffectInstance) { continue; }

        l_soundEffectInstance->Pause();
    }
}
void FWK::AudioManager::ResumeAllSound() const
{
    if (!m_audioEngine) { return; }

    for (const auto& l_soundEffectInstanceWeak : m_soundEffectInstanceList)
    {
        const auto& l_soundEffectInstance = l_soundEffectInstanceWeak.lock();

        if (!l_soundEffectInstance) { continue; }

        l_soundEffectInstance->Resume();
    }
}

void FWK::AudioManager::SaveCONFIG() const
{
    const auto& l_rootJson = m_jsonConverter.Serialize(*this);

    Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

void FWK::AudioManager::ApplyMasterVolume(const float a_volume)
{
    if (!m_audioEngine) { return; }

    // AudioEngine生成前にVolumeだけを設定された場合でも、
    // m_masterVolumeには値を保持しておく
    m_masterVolume = std::clamp(a_volume, k_minMasterVolume, k_maxMasterVolume);

    m_audioEngine->SetMasterVolume(m_masterVolume);
}

std::weak_ptr<FWK::SoundEffect> FWK::AudioManager::AddSoundEffect(const std::filesystem::path& a_filePath)
{
    if (!m_audioEngine) { return {}; }

    // もし同じFilePathのSoundEffectが既にマップに含まれていたら対応するSoundEffectを返す
    if (const auto& l_itr = m_soundEffectMap.find(a_filePath);
        l_itr != m_soundEffectMap.end())
    {
        return l_itr->second;
    }

    if (a_filePath.empty() ||
        !Utility::CanLoadFilePath(a_filePath, Constant::k_lowerWAVExtension))
    {
        return {};
    }

    const auto& l_soundEffect = std::make_shared<SoundEffect>();

    if (!l_soundEffect->Load(a_filePath)) { return {}; }

    m_soundEffectMap.try_emplace(a_filePath, l_soundEffect);

    return l_soundEffect;
}
void FWK::AudioManager::AddEffectInstance(const std::weak_ptr<SoundEffectInstanceBase>& a_soundEffectInstanceBase)
{
    if (a_soundEffectInstanceBase.expired()) { return; }

    m_soundEffectInstanceList.emplace_back(a_soundEffectInstanceBase);
}

void FWK::AudioManager::ClearMappedSound()
{
    // SoundEffectInstanceとSoundEffectのデータ消去前に全ての音の再生を止める
    StopAllSound();

    m_soundEffectInstanceList.clear();
    m_soundEffectMap.clear         ();
}

bool FWK::AudioManager::CreateAudioEngine()
{
    if (m_audioEngine) { return true; }

    // 3D音源では距離減衰や複数音源の合成によって
    // 最終出力がクリップする可能性があるため、
    // DirectXTK側のMasteringLimiterを有効にする
    auto l_audioEngineFlags = DirectX::AudioEngine_UseMasteringLimiter;

#if defined(_DEBUG)
    l_audioEngineFlags |= DirectX::AudioEngine_Debug;
#endif

    m_audioEngine = std::make_unique<DirectX::AudioEngine>(l_audioEngineFlags);

    FWK_ASSERT_RETURN_VALUE_IF(!m_audioEngine, "AudioEngineの作成に失敗しました。", false);

    m_audioEngine->SetMasterVolume(m_masterVolume);

    return true;
}

void FWK::AudioManager::RemoveSoundEffectInstanceIfStopped()
{
    // 破棄済み、または再生が停止したインスタンスは
    // 末尾要素と入れ替えてから削除する
    std::size_t l_index = k_initialRemoveSoundEffectInstanceIndex;

    while (l_index < m_soundEffectInstanceList.size())
    {
        // インスタンスが既に破棄されている、
        // または再生が停止している場合は管理リストから削除
        if (const auto& l_soundInstance = m_soundEffectInstanceList[l_index].lock();
            !l_soundInstance ||
            !l_soundInstance->IsStopped())
        {
            ++l_index;

            continue;
        }

        // 削除対象が末尾でなければ
        // 末尾要素を削除対象の位置へ移動する
        // 末尾から移動してきた要素を同じIndexでもう一度判定するため、l_indexは進めない
        if (l_index != m_soundEffectInstanceList.size() - k_soundEffectInstanceBeforeEndIndexOffset)
        {
            std::swap(m_soundEffectInstanceList[l_index], m_soundEffectInstanceList.back());
        }

        m_soundEffectInstanceList.pop_back();
    }
}

void FWK::AudioManager::Release()
{
    if (!m_audioEngine) { return; }

    // 全ての音を停止してリソースを解放
    ClearMappedSound();

    m_audioEngine.reset();
}