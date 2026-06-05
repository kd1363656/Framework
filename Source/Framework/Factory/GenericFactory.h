#pragma once

namespace FWK
{
	template <typename Type>
		requires Concept::IsSmartPTRConcept<Type>
	class GenericFactory final : public SingletonBase<GenericFactory<Type>>
	{
	private:

		using FactoryMap = std::unordered_map<std::string, std::function<Type()>, Struct::StringHash, std::equal_to<>>;

		// Typeはスマートポインタを想定している
		// std::shared_ptr<Base> / std::unique_ptr<Base>が管理している実体型Baseを取り出す
		using BaseType = typename Type::element_type;

	public:

		// "DerivedClass"をファクトリーに登録
		template <typename DerivedType>
			requires Concept::IsDerivedBaseConcept<DerivedType, BaseType>
		void Register(const std::string& a_typeName)
		{
			std::function<Type()> l_factoryMethod = {};

			// もしシェアードポインタ型ならシェアードポインタ型を
			// そうでないかつユニークポインタ型ならユニークポインタ型を
			// どちらにも該当しないなら"nullptr"を作るファクトリーを保存
			// "C++20"以降の"return"文は右辺値なら"RVO"が強制的に実行されるため所有権のコピーが発生しない
			if constexpr (TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Shared)
			{
				l_factoryMethod = []()
				{
					return std::make_shared<DerivedType>();
				};
			}
			else if constexpr (TypeTrait::PTRType<Type>::k_kind == Enum::PTRKind::Unique)
			{
				l_factoryMethod = []()
				{
					return std::make_unique<DerivedType>();
				};
			}

			// 登録
			m_factoryMap.try_emplace(a_typeName, l_factoryMethod);
		}

		Type Create(const std::string& a_className) const 
		{
			// マップから登録されているファクトリーメソッドを取得
			auto l_itr = m_factoryMap.find(a_className);

			if (l_itr == m_factoryMap.end()) { return nullptr; }
	
			return l_itr->second();
		}

	private:

		FactoryMap m_factoryMap = {};

		//=========================
		// シングルトン
		//=========================
		friend class SingletonBase<GenericFactory<Type>>;

		 GenericFactory()          = default;
		~GenericFactory() override = default;
	};
}