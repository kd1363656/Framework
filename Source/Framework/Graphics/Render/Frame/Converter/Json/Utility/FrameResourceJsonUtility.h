#pragma once

namespace FWK::Utility
{
	// 第1引数のjsonに第2引数のjsonの内容をコピーする
	inline void UpdateJson(nlohmann::json& a_targetJson, const nlohmann::json& a_patchJson)
	{
		if (a_patchJson.is_null()) { return; }

		a_targetJson.update(a_patchJson);
	}

	// ※注意 : FWK_REGISTER_FACTORY_METHODマクロを使用していないと使用できないので注意
	template <typename FactoryType , typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	inline void DeserializeInstanceType(const nlohmann::json& a_json, const std::string_view& a_key, Type& a_instance)
	{
		if (a_json.is_null()) { return; }

		const auto& l_factory = FactoryType::GetInstance();

		const auto& l_createName = a_json.value(a_key, std::string());

		a_instance = l_factory.Create(l_createName);
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