#pragma once

namespace FWK::Enum 
{
	enum class TextureUsageType
	{
		Auto,

		// BaseColor / Diffuseなど
		// 色として画面に出るため、基本的にSRGBとして扱う
		Color,

		// NormalMap
		// 色ではなくベクトルデータなので、SRGBにしてはいけない
		Normal,

		// Roughness / Metallic / AO / Mask / Heightなど。
		// 色ではなく数値データなので、Linearとして扱う
		Data,
	};
}