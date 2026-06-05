#pragma once

namespace FWK::TypeTrait
{
	// 型が生ポインタか、スマートポインタかを構造体の部分特殊化で判断する
	template <typename Type>
	struct PTRType
	{
		static constexpr Enum::PTRKind k_kind = Enum::PTRKind::None;
	};
	
	template <typename Type>
	struct PTRType<Type*>
	{
		static constexpr Enum::PTRKind k_kind = Enum::PTRKind::Raw;
	};

	template <typename Type>
	struct PTRType<std::shared_ptr<Type>>
	{
		static constexpr Enum::PTRKind k_kind = Enum::PTRKind::Shared;
	};
	
	template <typename Type>
	struct PTRType<std::unique_ptr<Type>>
	{
		static constexpr Enum::PTRKind k_kind = Enum::PTRKind::Unique;
	};
}