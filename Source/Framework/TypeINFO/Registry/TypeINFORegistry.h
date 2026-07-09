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

		// 文字列キー用の透過ハッシュ構造体
		struct StringHash final
		{
			// 透過ハッシュであることをSTLに通知するための宣言
			using is_transparent = void;
		
			// 受け取った文字列をstd::string_viewとして扱い、ハッシュ値を計算する
			std::size_t operator()(const std::string_view& a_key) const { return std::hash<std::string_view>{}(a_key); }
		};

	private:

		// k_nameは静的寿命であることを前提にstd::string_viewをキーとして使用
		using TypeINFONameMap         = std::unordered_map<std::string_view,        const TypeINFO* const, StringHash, std::equal_to<>>;
		using TypeINFOStaticTypeIDMap = std::unordered_map<TypeAlias::StaticTypeID, const TypeINFO* const>;

		friend class SingletonBase<FWK::TypeINFORegistry>;

		 TypeINFORegistry()          = default;
		~TypeINFORegistry() override = default;

	public:

		void Register(const TypeINFO& a_typeINFO);

		const TypeINFO* FindByName(const std::string_view&       a_name)         const;
		const TypeINFO* FindByID  (const TypeAlias::StaticTypeID a_staticTypeID) const;

	private:

		// 文字列がキーのマップはjsonのシリアライズ時に使用
		TypeINFONameMap         m_typeINFONameMap         = {};
		TypeINFOStaticTypeIDMap m_typeINFOStaticTypeIDMap = {};
	};
}