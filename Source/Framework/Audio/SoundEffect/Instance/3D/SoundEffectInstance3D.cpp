#include "SoundEffectInstance3D.h"

bool FWK::SoundEffectInstance3D::CreateInstance(const std::weak_ptr<SoundEffect>& a_soundEffect)
{
	if (!SoundEffectInstanceBase::CreateInstance(a_soundEffect)) { return false; }

	// Apply3D()を使用するため
	// SoundEffectInstance_Use3Dを指定して生成する
	return SoundEffectInstanceBase::CreateInstance(k_createInstanceFlags);
}

void FWK::SoundEffectInstance3D::ApplyPosition(const TypeAlias::Math::Vector3& a_position)
{
	// 3D空間上における音源の位置をEmitterへ設定する
	m_emitter.SetPosition(a_position);
}
void FWK::SoundEffectInstance3D::ApplyVelocity(const TypeAlias::Math::Vector3& a_velocity)
{
	// 音源の移動速度
	m_emitter.SetVelocity(a_velocity);
}
void FWK::SoundEffectInstance3D::ApplyOrientation(const TypeAlias::Math::Vector3& a_forward, const TypeAlias::Math::Vector3& a_up)
{
	// 音自身が向いている方向を設定する
	// 指向性を持つ音源などで利用
	m_emitter.SetOrientation(a_forward, a_up);
}
void FWK::SoundEffectInstance3D::ApplyCurveDistanceScaler(const float a_value)
{
	m_emitter.CurveDistanceScaler = a_value;
}
void FWK::SoundEffectInstance3D::Apply3D(const DirectX::AudioListener& a_listener)
{
	const auto& l_instance = GetREFInstance();

	if (!l_instance) { return; }

	// ListenerとEmitterの位置・速度・向きなどを元に、
	// DirectXTK側で3D音響計算を行ってInstanceへ反映する
	l_instance->Apply3D(a_listener, m_emitter, k_isRightHandedCoordinates);
}