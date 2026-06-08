#pragma once

namespace FWK::Utility
{
	inline std::string WStringToString(const std::wstring& a_wString)
	{
		if (a_wString.empty()) { return {}; }

		// UTF-16文字列(std::wstring)をUTF-8文字列(std::string)へ変換するために、
		// まず変換後に必要になるバッファサイズを取得する
		// WideCharToMultiByte
		// (
		//		変換の実行に使用するコードページ（UTF-8など）,
		//		変換フラグ,
		//		変換元ワイド文字列の先頭ポインタ,
		//		変換元ワイド文字列の文字数,
		//		出力用に格納するマルチバイト文字列の先頭アドレス（nullptr = サイズ問い合わせ）,
		//		出力用マルチバイト文字列のサイズ				  （0       = サイズ取得モード）,
		//		変換できない文字があった場合に使う代替文字		  （nullptr = 既定値を使う）,
		//		代替文字が使われたかどうかを受け取るフラグ		  （nullptr = 受け取らない）
		// )
		const int l_size = WideCharToMultiByte(CP_UTF8,
											   Constant::k_wCharToMultiByteFlags,
											   a_wString.data(),
											   static_cast<int>(a_wString.size()),
											   nullptr,
											   Constant::k_wCharToMultiByteQueryBufferSize,
											   nullptr,
											   nullptr);

		FWK_ASSERT_RETURN_VALUE_IF_FAILED(l_size == Constant::k_invalidConvertedMultiByteSize, "WideCharToMultiByte(UTF-16からUTF-8への変換)に失敗しました。", {});

		// 変換後のUTF-8文字列を格納するためのバッファを確保する
		// l_sizeには必要な文字数が入っているため、そのサイズ分だけstd::stringを作成する
		std::string l_result(l_size, Constant::k_nullCharacter);

		// UTF-16 -> UTF-8へ実データ変換
		// 1回目のWideCharToMultiByteではサイズ取得だけを行い、
		// 2回目のWideCharToMultiByteで実際にl_resultへ文字列を書き込む
		WideCharToMultiByte(CP_UTF8,
							Constant::k_wCharToMultiByteFlags,
							a_wString.data(),
							static_cast<int>(a_wString.size()),
							l_result.data(),
							l_size,
							nullptr,
							nullptr);

		return l_result;
	}
}