#include "SoundEffectInstanceBase.h"

FWK::SoundEffectInstanceBase::SoundEffectInstanceBase() : 
	m_instance(nullptr),

	m_soundEffect()
{}
FWK::SoundEffectInstanceBase::~SoundEffectInstanceBase()
{
	// 再生を注視してから
	Stop();

	m_instance.reset();
}

bool FWK::SoundEffectInstanceBase::CreateInstance(const std::weak_ptr<SoundEffect>& a_soundEffect)
{
	if (a_soundEffect.expired()) { return false; }

	// 既存のInstanceは以前設定されていたSoundEffectの音声データを参照してる可能性があるため先に破棄する
	m_instance.reset();

	m_soundEffect = a_soundEffect;

	return true;
}

void FWK::SoundEffectInstanceBase::Play(const bool a_isLoop)
{
	if (!m_instance) { return; }

	// 現在が再生中・一時停止中など、どの状態からPlay()が呼ばれても
	// 音声を先頭から再生できる用に、一度停止して再生位置をリセットする
	Stop();

	m_instance->Play(a_isLoop);
}

void FWK::SoundEffectInstanceBase::Stop()
{
	if (!m_instance) { return; }

	m_instance->Stop();
}

void FWK::SoundEffectInstanceBase::Pause()
{
	if (!m_instance) { return; }

	m_instance->Pause();
}

void FWK::SoundEffectInstanceBase::Resume()
{
	if (!m_instance) { return; }

	m_instance->Resume();
}

void FWK::SoundEffectInstanceBase::ApplyVolume(const float a_volume)
{
	if (!m_instance) { return; }

	// DirectXTKのVolumeとして扱える範囲へ制限する
	const float l_volume = std::clamp(a_volume, Constant::k_soundEffectInstanceMINVolume, Constant::k_soundEffectInstanceMAXVolume);

	m_instance->SetVolume(l_volume);
}
void FWK::SoundEffectInstanceBase::ApplyPitch(const float a_pitch)
{
	if (!m_instance) { return; }

	// DirectXTKのVolumeとして扱える範囲へ制限する
	const float l_pitch = std::clamp(a_pitch, Constant::k_soundEffectInstanceMINPitch, Constant::k_soundEffectInstanceMAXPitch);

	m_instance->SetPitch(l_pitch);
}

bool FWK::SoundEffectInstanceBase::IsPlaying() const
{
	if (!m_instance) { return false; }

	return m_instance->GetState() == DirectX::SoundState::PLAYING;
}
bool FWK::SoundEffectInstanceBase::IsPause() const
{
	if (!m_instance) { return false; }

	return m_instance->GetState() == DirectX::SoundState::PAUSED;
}
bool FWK::SoundEffectInstanceBase::IsStopped() const
{
	if (!m_instance) { return false; }

	return m_instance->GetState() == DirectX::SoundState::STOPPED;
}

bool FWK::SoundEffectInstanceBase::CreateInstance(const DirectX::SOUND_EFFECT_INSTANCE_FLAGS a_flags)
{
	// 同じラッパーに対して二重にDirectXTK側Instanceを生成しない
	// 既に存在するなら作成済みとして成功扱いにする
	if (m_instance) { return true; }

	const auto& l_soundEffect = m_soundEffect.lock();

	// SoundEffectが存在しなければ生成元となる音声データがないため
	// DirectXTK側Instanceも作成できない
	if (!l_soundEffect) { return false; }

	m_instance = l_soundEffect->CreateInstance(a_flags);

	return true;
}