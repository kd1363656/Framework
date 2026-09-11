#pragma once

namespace FWK
{
	class SoundEffectInstance2D final : public SoundEffectInstanceBase
	{
	public:
 
		 SoundEffectInstance2D()          = default;
		~SoundEffectInstance2D() override = default;

		bool CreateInstance(const std::weak_ptr<SoundEffect>& a_soundEffect) override;

		void ApplyPan(const float a_pan);

	private:

		static constexpr DirectX::SOUND_EFFECT_INSTANCE_FLAGS k_createInstanceFlags = DirectX::SoundEffectInstance_Default;
	};
}