#pragma once

namespace FWK::Enum
{
	enum class TextureLoadColorSpace
	{
		// DirectXTex/WIC側の標準判定に任せる
		Auto,

		// 色として扱うテクスチャ
		// BaseColor / Albedo / Diffuse / EmissiveColorなど
		SRGB,

		// 色ではなく、シェーダー計算用の数値データーとして扱うテクスチャ
		// Normal / Roughness / Metallic / AO / Maskなど
		Linear,
	};
}