#pragma once

namespace FWK::Utility
{
	// 読み込んだjsonが配列かどうかを確認
	inline bool IsJsonArray(const nlohmann::json& a_json, const std::string_view& a_key = {})
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

	// 第1引数のjsonに第2引数のjsonの内容をコピーする
	inline void UpdateJson(nlohmann::json& a_targetJson, const nlohmann::json& a_patchJson)
	{
		if (a_patchJson.is_null()) { return; }

		a_targetJson.update(a_patchJson);
	}

	inline TypeAlias::Math::Color DeserializeColor(const nlohmann::json& a_json , const std::string_view& a_key)
	{
		// "json"を読み込めるか確認、読み込めなければ"return"
		const auto& l_json = a_json.value(a_key, nlohmann::json{});

		if (l_json.is_null()) { return {}; }

		return TypeAlias::Math::Color
		{
			l_json.value(Constant::k_colorRJsonKey, Constant::k_whiteColor.R()),
			l_json.value(Constant::k_colorGJsonKey, Constant::k_whiteColor.G()),
			l_json.value(Constant::k_colorBJsonKey, Constant::k_whiteColor.B()),
			l_json.value(Constant::k_colorAJsonKey, Constant::k_whiteColor.A())
		};
	}

	inline TypeAlias::Math::Vector3 DeserializeVector3(const nlohmann::json& a_json , const std::string_view a_key)
	{
		// "json"を読み込めるか確認、読み込めなければ"return"
		if (a_json.is_null() || 
			a_key.empty()    ||
			!a_json.contains(a_key.data()))
		{
			return {};
		}

		const auto& l_json = a_json[a_key.data()];

		if (l_json.is_null())  { return {}; }

		return TypeAlias::Math::Vector3
		{
			l_json.value(Constant::k_xJsonKey, TypeAlias::Math::Vector3::Zero.x),
			l_json.value(Constant::k_yJsonKey, TypeAlias::Math::Vector3::Zero.y),
			l_json.value(Constant::k_zJsonKey, TypeAlias::Math::Vector3::Zero.z)
		};
	}

	inline TypeAlias::Math::Quaternion DeserializeQuaternion(const nlohmann::json& a_json , const std::string_view a_key)
	{
		// "json"を読み込めるか確認、読み込めなければ"return"
		if (a_json.is_null() || 
			a_key.empty()    ||
			!a_json.contains(a_key.data()))
		{
			return {};
		}

		const auto& l_json = a_json[a_key.data()];

		if (l_json.is_null()) { return {}; }

		return TypeAlias::Math::Quaternion
		{
			l_json.value(Constant::k_xJsonKey, TypeAlias::Math::Quaternion::Identity.x),
			l_json.value(Constant::k_yJsonKey, TypeAlias::Math::Quaternion::Identity.y),
			l_json.value(Constant::k_zJsonKey, TypeAlias::Math::Quaternion::Identity.z),
			l_json.value(Constant::k_wJsonKey, TypeAlias::Math::Quaternion::Identity.w)
		};
	}

	// ※注意 : FWK_REGISTER_FACTORY_METHODマクロを使用していないと使用できないので注意
	template <typename FactoryType, typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	inline void DeserializeInstanceType(const nlohmann::json& a_json, const std::string_view& a_key, Type& a_instance)
	{
		if (a_json.is_null()) { return; }

		const auto& l_factory = FactoryType::GetInstance();

		const auto& l_createName = a_json.value(a_key, std::string());

		a_instance = l_factory.Create(l_createName);
	}

	inline nlohmann::json SerializeColor(const TypeAlias::Math::Color& a_color , const std::string_view& a_key)
	{
		// キーとなる文字列がなければ空の"json"を返す
		if (a_key.empty()) { return {}; }

		return nlohmann::json
		{
			{
				a_key,
				{
					{ Constant::k_colorRJsonKey, a_color.R() },
					{ Constant::k_colorGJsonKey, a_color.G() },
					{ Constant::k_colorBJsonKey, a_color.B() },
					{ Constant::k_colorAJsonKey, a_color.A() },
				}
			}
		};
	}

	inline nlohmann::json SerializeVector3(const TypeAlias::Math::Vector3& a_value , const std::string_view a_key)
	{
		// キーとなる文字列がなければ空の"json"を返す
		if (a_key.empty()) { return nlohmann::json(); }

		return nlohmann::json
		{
			{
				a_key.data() ,
				{
					{ Constant::k_xJsonKey, a_value.x } ,
					{ Constant::k_yJsonKey, a_value.y } ,
					{ Constant::k_zJsonKey, a_value.z }
				}
			}
		};
	}

	inline nlohmann::json SerializeQuaternion(const TypeAlias::Math::Quaternion& a_value , const std::string_view a_key)
	{
		// キーとなる文字列がなければ空の"json"を返す
		if (a_key.empty()) { return {}; }

		return nlohmann::json
		{
			{
				a_key.data() ,
				{
					{ Constant::k_xJsonKey, a_value.x } ,
					{ Constant::k_yJsonKey, a_value.y } ,
					{ Constant::k_zJsonKey, a_value.z } ,
					{ Constant::k_wJsonKey, a_value.w } ,
				}
			}
		};
	}

	// インスタンスから型名を取得し保存する
	// ※注意 : FWK_REGISTER_FACTORY_METHODマクロを使用していないと使用できないので注意
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	inline nlohmann::json SerializeInstanceType(const Type& a_instance, const std::string_view& a_key)
	{
		if (!a_instance) { return nlohmann::json(); }

		auto l_rootJson = nlohmann::json();

		l_rootJson[a_key] = a_instance->GetREFRuntimeTypeINFO().k_name;

		return l_rootJson;
	}
}