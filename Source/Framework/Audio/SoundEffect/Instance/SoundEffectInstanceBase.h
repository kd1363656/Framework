#pragma once

namespace FWK
{
	class SoundEffectInstanceBase
	{
	public:

	             SoundEffectInstanceBase() = default;
		virtual ~SoundEffectInstanceBase() = default;

		virtual bool CreateInstance(const std::weak_ptr<SoundEffect>& a_soundEffect);

		void Play(const bool a_isLoop = false);

		void Stop();

		void Pause();

		void Resume();

		void ApplyVolume(const float a_volume);
		void ApplyPitch (const float a_pitch);

		bool IsPlaying() const;
		bool IsPause  () const;
		bool IsStopped() const;

	protected:

		bool CreateInstance(const DirectX::SOUND_EFFECT_INSTANCE_FLAGS a_flags);

		const auto& GetREFInstance() const { return m_instance; }

	private:

		std::unique_ptr<DirectX::SoundEffectInstance> m_instance = nullptr;

		std::weak_ptr<SoundEffect> m_soundEffect = {};
	};
}