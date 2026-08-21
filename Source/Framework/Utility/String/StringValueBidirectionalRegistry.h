#pragma once

namespace FWK::Utility
{
	template <typename Type>
	class StringValueBidirectionalRegistry final : public SingletonBase<StringValueBidirectionalRegistry<Type>>
	{
	private:

		using StringToValueMap = std::unordered_map<std::string, Type, Struct::StringHashStruct, std::equal_to<>>;
		using ValueToStringMap = std::unordered_map<Type,        std::string>;

		friend class SingletonBase<StringValueBidirectionalRegistry<Type>>;

		 StringValueBidirectionalRegistry()          = default;
		~StringValueBidirectionalRegistry() override = default;

	public:

		void Register(const Type& a_type, const std::string& a_key)
		{
			FWK_ASSERT_RETURN_IF(a_key.empty(), "文字列が空になっており、値の登録に失敗しました。");

			bool l_isFailedRegister = m_stringToValueMap.try_emplace(a_key, a_type).second;
			
			l_isFailedRegister = m_valueToStringMap.try_emplace(a_type, a_key).second;

			FWK_ASSERT_RETURN_IF(!l_isFailedRegister, "登録する際のキーが重複しており、値の登録に失敗しました。");
		}

		const Type FindVALValueByKey(const std::string_view& a_key)
		{
			const auto& l_itr = m_stringToValueMap.find(a_key);

			// 該当する名前の値を取得できなければreturn;
			if (l_itr == m_stringToValueMap.end()) { return nullptr; }

			return l_itr->second;
		}

		const std::string_view FindVALKeyByValue(const Type a_type)
		{
			const auto& l_itr = m_valueToStringMap.find(a_type);

			// 該当する名前の値を取得できなければreturn;
			if (l_itr == m_valueToStringMap.end()) { return nullptr; }

			return l_itr->second;
		}

		const auto& GetREFStringToValueMap() const { return m_stringToValueMap; }

		StringToValueMap m_stringToValueMap = {};
		ValueToStringMap m_valueToStringMap = {};
	};
}