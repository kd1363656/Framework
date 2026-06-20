#pragma once

namespace FWK::Utility
{
	inline TypeAlias::Math::Color DeserializeColor(const nlohmann::json& a_json , const std::string_view a_key)
	{
		// "json"を読み込めるか確認、読み込めなければ"return"
		const auto& l_json = a_json.value(a_key, nlohmann::json{});

		if (l_json.is_null()) { return {}; }

		return TypeAlias::Math::Color
		{
			l_json.value("R" , Constant::k_colorLess.R()),
			l_json.value("G" , Constant::k_colorLess.G()),
			l_json.value("B" , Constant::k_colorLess.B()),
			l_json.value("A" , Constant::k_colorLess.A())
		};
	}

	inline nlohmann::json SerializeColor(const TypeAlias::Math::Color& a_color , const std::string_view a_key)
	{
		// キーとなる文字列がなければ空の"json"を返す
		if (a_key.empty()) { return {}; }

		return nlohmann::json
		{
			{
				a_key,
				{
					{ "R" , a_color.R() },
					{ "G" , a_color.G() },
					{ "B" , a_color.B() },
					{ "A" , a_color.A() },
				}
			}
		};
	}
}