#pragma once

namespace FWK
{
	class SoundEffectInstance3D final : public SoundEffectInstanceBase
	{
	public:
 
		 SoundEffectInstance3D()          = default;
		~SoundEffectInstance3D() override = default;

		bool CreateInstance(const std::weak_ptr<SoundEffect>& a_soundEffect) override;

		void ApplyPosition           (const TypeAlias::Math::Vector3& a_position);
		void ApplyVelocity           (const TypeAlias::Math::Vector3& a_velocity);
		void ApplyOrientation        (const TypeAlias::Math::Vector3& a_forward, const TypeAlias::Math::Vector3& a_up);
		void ApplyCurveDistanceScaler(const float                     a_value);
		void Apply3D                 (const DirectX::AudioListener&   a_listener);

	private:

		static constexpr DirectX::SOUND_EFFECT_INSTANCE_FLAGS k_createInstanceFlags = DirectX::SoundEffectInstance_Use3D;

		static constexpr bool k_isRightHandedCoordinates = false;

		DirectX::AudioEmitter m_emitter = {};
	};
}