#pragma once

namespace FWK
{
	class SoundEffect
	{
	public:

		 SoundEffect() = default;
		~SoundEffect() = default;

		std::unique_ptr<DirectX::SoundEffectInstance> CreateInstance(DirectX::SOUND_EFFECT_INSTANCE_FLAGS a_flags);

		bool Load(const std::filesystem::path& a_filePath);

	private:

		std::unique_ptr<DirectX::SoundEffect> m_soundEffect;
	};
}