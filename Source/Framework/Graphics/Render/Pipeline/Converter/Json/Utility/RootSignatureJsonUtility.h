#pragma once

namespace FWK::Utility
{
	// 読み込んだjsonが配列かどうかを確認
	inline bool IsArray(const nlohmann::json& a_json, const std::string_view& a_key = {})
	{
		// jsonが空ならreturn
		if (a_json.is_null()) { return false; }

		// 文字列が空ならa_jsonが配列かどうかの結果を返す
		if (a_key.empty()) { return a_json.is_array(); }

		// もしキーが含まれているがa_jsonに含まれていな得ればreturn
		if (!a_json.contains(a_key)) { return false; }

		// キーが含まれていればa_json[a_key.data()]が配列なのかどうかの結果を返す
		return a_json[a_key].is_array();
	}
}