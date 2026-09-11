#include "SoundEffectInstance2D.h"

bool FWK::SoundEffectInstance2D::CreateInstance(const std::weak_ptr<SoundEffect>& a_soundEffect)
{
	// Base側へSoundEffectを設定する
	// 無効なweak_ptrだった場合はInstanceを生成できない
	if (!SoundEffectInstanceBase::CreateInstance(a_soundEffect)) { return false; }

	// 2D用の生成フラグでDirectXTK側Instanceを生成する
	return SoundEffectInstanceBase::CreateInstance(k_createInstanceFlags);
}

void FWK::SoundEffectInstance2D::ApplyPan(const float a_pan)
{
	const auto& l_instance = GetREFInstance();

	if (!l_instance) { return; }

	// DirectXTKのPanとして扱える範囲へ制限する
	const float l_pan = std::clamp(a_pan, Constant::k_soundEffectInstance2DMINPan, Constant::k_soundEffectInstance2DMAXPan);

	l_instance->SetPan(l_pan);
}