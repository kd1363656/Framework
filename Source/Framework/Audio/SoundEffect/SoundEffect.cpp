#include "SoundEffect.h"

std::unique_ptr<DirectX::SoundEffectInstance> FWK::SoundEffect::CreateInstance(const DirectX::SOUND_EFFECT_INSTANCE_FLAGS a_flags)
{
    if (!m_soundEffect) { return nullptr; }

    return m_soundEffect->CreateInstance(a_flags);
}

bool FWK::SoundEffect::Load(const std::filesystem::path& a_filePath)
{
    if (a_filePath.empty() ||
        !Utility::CanLoadFilePath(a_filePath, Constant::k_lowerWAVExtension))
    {
        return false;
    }

    const auto& l_audioManager = AudioManager::GetInstance       ();
    const auto& l_audioEngine  = l_audioManager.GetREFAudioEngine();

    m_soundEffect = std::make_unique<DirectX::SoundEffect>(l_audioEngine.get(), a_filePath.c_str());

    if (!m_soundEffect) { return false; }

    return true;
}