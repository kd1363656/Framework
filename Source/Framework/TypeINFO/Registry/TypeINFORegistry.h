#pragma once

namespace FWK
{
	// 型情報を指定しなくてもTypeInfoを取得できるようにするためのレジストリークラス
	class TypeINFORegistry final : public Utility::SingletonBase<TypeINFORegistry>
	{
	private:

		// k_nameは静的寿命であることを前提にstd::string_viewをキーとして使用
		using TypeINFONameMap         = std::unordered_map<std::string_view,        const Struct::TypeINFO* const, Struct::StringHash, std::equal_to<>>;
		using TypeINFOStaticTypeIDMap = std::unordered_map<TypeAlias::StaticTypeID, const Struct::TypeINFO* const>;

		friend class SingletonBase<FWK::TypeINFORegistry>;

		 TypeINFORegistry()          = default;
		~TypeINFORegistry() override = default;

	public:

		void Register(const Struct::TypeINFO& a_typeINFO);

		const Struct::TypeINFO* FindByName(const std::string_view&       a_name)         const;
		const Struct::TypeINFO* FindByID  (const TypeAlias::StaticTypeID a_staticTypeID) const;

	private:

		// 文字列がキーのマップはjsonのシリアライズ時に使用
		TypeINFONameMap         m_typeINFONameMap         = {};
		TypeINFOStaticTypeIDMap m_typeINFOStaticTypeIDMap = {};
	};
}