#pragma once

namespace FWK
{
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	class GenericFactory final : public Utility::SingletonBase<GenericFactory<Type>>
	{
	private:

		using FactoryMap = std::unordered_map<std::string, std::function<Type()>, Struct::StringHash, std::equal_to<>>;

		// Typeはスマートポインタを想定している
		// std::shared_ptr<Base> / std::unique_ptr<Base>が管理している実体型Baseを取り出す
		using BaseType = typename Type::element_type;

		//=========================
		// シングルトン
		//=========================
		friend class Utility::SingletonBase<GenericFactory<Type>>;

		 GenericFactory()          = default;
		~GenericFactory() override = default;

		static constexpr bool k_isSupportedPTR = TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared ||
												 TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Unique;

		static_assert(k_isSupportedPTR, "GenericFactoryはstd::shared_ptrまたはstd::unique_ptrだけに対応しています");

	public:

		// "DerivedClass"をファクトリーに登録
		template <typename DerivedType>
			requires Concept::IsDerivedBaseConcept<DerivedType, BaseType>
		void Register(const std::string& a_typeName)
		{
			// 作成処理の登録もしTypeがstd::shared_ptrならstd::shared_ptr番のCreateInstanceが、
			// そうでないならstd::unique_ptr番のCreateInstanceがコンパイル時に選択される
			m_factoryMap.try_emplace(a_typeName, []() -> Type 
			{
				return CreateInstance<DerivedType>();
			});
		}

		Type Create(const std::string& a_className) const 
		{
			// マップから登録されているファクトリーメソッドを取得
			auto l_itr = m_factoryMap.find(a_className);

			if (l_itr == m_factoryMap.end()) { return {}; }
	
			return l_itr->second();
		}

		const auto& GetREFFactoryMap() const { return m_factoryMap; }

	private:

		// Typeがshared_ptrの場合にだけ使用可能になる。
		template <typename DerivedType>
			requires (TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared)
		static Type CreateInstance()
		{
			return std::make_shared<DerivedType>();
		}

		// Typeがunique_ptrの場合にだけ使用可能になる。
		template <typename DerivedType>
			requires (TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Unique)
		static Type CreateInstance()
		{
			return std::make_unique<DerivedType>();
		}

		FactoryMap m_factoryMap = {};
	};
}