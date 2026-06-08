#pragma once

namespace FWK::Utility
{
	// 第1引数のjsonに第2引数のjsonの内容をコピーする
	inline void UpdateJson(nlohmann::json& a_targetJson, const nlohmann::json& a_patchJson)
	{
		if (a_patchJson.is_null()) { return; }

		a_targetJson.update(a_patchJson);
	}

	inline TypeAlias::Math::Color DeserializeColor(const nlohmann::json& a_json, const std::string_view a_key)
	{
		// "json"を読み込めるか確認、読み込めなければ"return"
		if (a_json.is_null() || a_key.empty() || !a_json.contains(a_key.data())) { return {}; }

		const auto& l_json = a_json.value(a_key, nlohmann::json{});

		if (l_json.is_null()) { return {}; }

		return TypeAlias::Math::Color
		{
			l_json.value(Constant::k_colorRJsonKey, Constant::k_colorLess.x) ,
			l_json.value(Constant::k_colorGJsonKey, Constant::k_colorLess.y) ,
			l_json.value(Constant::k_colorBJsonKey, Constant::k_colorLess.z) ,
			l_json.value(Constant::k_colorAJsonKey, Constant::k_colorLess.w)
		};
	}

	inline nlohmann::json SerializeColor(const TypeAlias::Math::Color& a_value , const std::string_view a_key)
	{
		// キーとなる文字列がなければ空の"json"を返す
		if (a_key.empty()) { return {}; }

		return nlohmann::json
		{
			{
				a_key.data(),
				{
					{ Constant::k_colorRJsonKey , a_value.x } ,
					{ Constant::k_colorGJsonKey , a_value.y } ,
					{ Constant::k_colorBJsonKey , a_value.z } ,
					{ Constant::k_colorAJsonKey , a_value.w } ,
				}
			}
		};
	}	
}