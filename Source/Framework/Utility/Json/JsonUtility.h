#pragma once

namespace FWK::Utility
{
	// 第1引数のjsonに第2引数のjsonの内容をコピーする
	inline void UpdateJson(nlohmann::json& a_targetJson, const nlohmann::json& a_patchJson)
	{
		if (a_patchJson.is_null()) { return; }

		a_targetJson.update(a_patchJson);
	}

	// 読み込んだjsonが配列かどうかを確認
	inline bool IsArray(const nlohmann::json& a_json, const std::string_view& a_key = {})
	{
		// jsonが空ならreturn
		if (a_json.is_null()) { return false; }

		// 文字列が空ならa_jsonが配列かどうかの結果を返す
		if (a_key.empty()) { return a_json.is_array(); }

		// もしキーが含まれているがa_jsonに含まれていなければreturn
		if (!a_json.contains(a_key)) { return false; }

		// キーが含まれていればa_json[a_key.data()]が配列なのかどうかの結果を返す
		return a_json[a_key].is_array();
	}

	inline TypeAlias::TagType DeserializeTag(const nlohmann::json& a_json, const std::string_view& a_key)
	{
		if (a_json.is_null()) { return Constant::k_invalidStaticTypeID; }

		// jsonからTag名を取得
		const auto& l_registry = TypeINFORegistry::GetInstance();
		const auto& l_tagName  = a_json.value                 (a_key, std::string());

		// 取得したタグ名から型情報を検索
		const auto* l_typeINFO = l_registry.FindByName(l_tagName);

		// もし型情報がnullptrを示すということはTypeTagの取得に失敗したということ
		if (!l_typeINFO) { return Constant::k_invalidStaticTypeID; }

		return l_typeINFO->k_staticTypeID;
	}

	inline nlohmann::json SerializeTag(const TypeAlias::TagType  a_tagType, const std::string_view& a_key)
	{
		const auto& l_registry = TypeINFORegistry::GetInstance();
		const auto* l_typeINFO = l_registry.FindByID          (a_tagType);

		if (!l_typeINFO)
		{
			return nlohmann::json
			{ 
				{ a_key, std::string() } 
			};
		}

		return nlohmann::json
		{
			{ a_key, l_typeINFO->k_name }
		};
	}
}