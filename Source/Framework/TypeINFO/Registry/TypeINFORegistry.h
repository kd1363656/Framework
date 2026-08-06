#pragma once

namespace FWK
{
	// 型情報を指定しなくてもTypeInfoを取得できるようにするためのレジストリークラス
	class TypeINFORegistry final : public Utility::SingletonBase<TypeINFORegistry>
	{
	public:

		// 型情報を扱う構造体
		struct TypeINFO final
		{
			explicit TypeINFO(const TypeINFO* const a_baseINFO, const std::string_view& a_name, const TypeAlias::StaticTypeID a_staticTypeID) :
				k_baseINFO    (a_baseINFO),
				k_name        (a_name),
				k_staticTypeID(a_staticTypeID)
			{}
			~TypeINFO() = default;

			TypeINFO(const TypeINFO&)  = delete;
			TypeINFO(	   TypeINFO&&) = delete;

			TypeINFO& operator=(const TypeINFO&)  = delete;
			TypeINFO& operator=(	  TypeINFO&&) = delete;

			const TypeINFO* const         k_baseINFO;
			const std::string_view        k_name;
			const TypeAlias::StaticTypeID k_staticTypeID;
		};

	private:

		// k_nameは静的寿命であることを前提にstd::string_viewをキーとして使用
		using TypeINFONameMap         = std::unordered_map<std::string_view,        const TypeINFO* const, Struct::StringHashStruct, std::equal_to<>>;
		using TypeINFOStaticTypeIDMap = std::unordered_map<TypeAlias::StaticTypeID, const TypeINFO* const>;

		friend class SingletonBase<TypeINFORegistry>;

		 TypeINFORegistry()          = default;
		~TypeINFORegistry() override = default;

	public:

		void Register(const TypeINFO& a_typeINFO);

		const TypeINFO* FindPTRByName(const std::string_view&       a_name)         const;
		const TypeINFO* FindPTRByID  (const TypeAlias::StaticTypeID a_staticTypeID) const;

		const auto& GetREFTypeINFONameMap() const { return m_typeINFONameMap; }

	private:

		// 文字列がキーのマップはjsonのシリアライズ時に使用
		TypeINFONameMap         m_typeINFONameMap         = {};
		TypeINFOStaticTypeIDMap m_typeINFOStaticTypeIDMap = {};
	};
}