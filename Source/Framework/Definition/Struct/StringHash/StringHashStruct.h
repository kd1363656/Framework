#pragma once

namespace FWK::Struct
{
	// 文字列キー用の透過ハッシュ構造体
	struct StringHash final
	{
		// 透過ハッシュであることをSTLに通知するための宣言
		using is_transparent = void;
	
		// 受け取った文字列をstd::string_viewとして扱い、ハッシュ値を計算する
		std::size_t operator()(const std::string_view& a_key) const { return std::hash<std::string_view>{}(a_key); }
	};

	// ワイド文字列キー用の等価ハッシュ構造体
	struct WStringHash final
	{
		// 透過ハッシュであることをSTLに通知するための宣言
		using is_transparent = void;
	
		// 受け取った文字列をstd::wstring_viewとして扱い、ハッシュ値を計算する
		std::size_t operator()(const std::wstring_view& a_key) const { return std::hash<std::wstring_view>{}(a_key); }
	};
}