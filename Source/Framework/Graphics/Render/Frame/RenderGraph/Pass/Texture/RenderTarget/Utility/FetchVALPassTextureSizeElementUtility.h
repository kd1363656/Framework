#pragma once

namespace FWK::Utility
{
	// ※ 注意
	// レンダーターゲットテクスチャ、デプスステンシルテクスチャにしか使わない想定の便利関数
	inline UINT FetchVALPassTextureSizeElement(const UINT a_clientSizeElement, const UINT a_fixedTextureSizeElement, const bool a_isFixedSize)
	{
		// 固定サイズを使う設定なら、JSONなどで保存されたTexture側のサイズを使う
		if (a_isFixedSize) { return a_fixedTextureSizeElement; }

		// 固定サイズでいいなら、WindowのClientSizeを返す
		return a_clientSizeElement;
	}
}