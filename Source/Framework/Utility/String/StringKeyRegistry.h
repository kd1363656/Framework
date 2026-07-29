#pragma once

namespace FWK::Utility
{
	template <typename Type>
	class StringKeyRegistry final : public SingletonBase<StringKeyRegistry<Type>>
	{
	private:

		using StringToValueMap = std::unordered_map<std::string, Type, Struct::StringHash, std::equal_to<>>;
		using ValueToStringMap = std::unordered_map<Type,        std::string>;

		friend class SingletonBase<StringKeyRegistry<Type>>;

		 StringKeyRegistry()          = default;
		~StringKeyRegistry() override = default;

		void Register(const Type& a_type, const std::string& a_key)
		{
			FWK_ASSERT_RETURN_IF(a_key.empty(), "文字列が空になっており、値の登録に失敗しました。");

			const bool l_isFailedRegister = m_stringToValuemap.try_emplace(a_key, a_type);

			FWK_ASSERT_RETURN_IF(!l_isFailedRegister, "登録する際のキーが重複しており、値の登録に失敗しました。");
		}

		const Type FindVALByKey(const std::string_view& a_key)
		{
			const auto& l_itr = m_stringToValueMap.find(a_key);

			// 該当する名前の値を取得できなければreturn;
			if (l_itr == m_stringToValueMap.end()) { return nullptr; }

			return l_itr->second;
		}

		const std::string_view FindVALByValue(const Type a_type)
		{
			const auto& l_itr = m_valueToStringMap.find(a_type);

			// 該当する名前の値を取得できなければreturn;
			if (l_itr == m_valueToStringMap.end()) { return nullptr; }

			return l_itr->second;
		}

		const auto& GetREFStringToValueMap() const { return m_stringToValueMap; }

	public:

		StringToValueMap m_stringToValueMap = {};
		ValueToStringMap m_valueToStringMap = {};
	};
}