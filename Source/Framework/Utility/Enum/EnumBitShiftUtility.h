#pragma once

namespace FWK::Utility
{
	template <typename EnumType, typename FlagType>
		requires std::is_enum_v    <EnumType> &&
	             std::is_unsigned_v<FlagType>
	inline FlagType EnableFlag(EnumType a_enableFlag, FlagType a_flags)
	{
		const auto l_enableFlag = static_cast<FlagType>(a_enableFlag);

		return a_flags | l_enableFlag;
	}

	template <typename EnumType, typename FlagType>
		requires std::is_enum_v    <EnumType> &&
	             std::is_unsigned_v<FlagType>
	inline FlagType DisableFlag(EnumType a_disableFlag, FlagType a_flags)
	{
		const auto l_disableFlag = static_cast<FlagType>(a_disableFlag);

		return a_flags & ~l_disableFlag;
	}

	template <typename EnumType, typename FlagType>
		requires std::is_enum_v    <EnumType> &&
	             std::is_unsigned_v<FlagType>
	inline FlagType EnableOnlyFlag(EnumType a_enableFlag, FlagType a_flags)
	{
		// 全てのフラグを初期化
		a_flags = static_cast<FlagType>(Constant::k_noFlagValue);

		return EnableFlag(a_enableFlag, a_flags);
	}

	template <typename EnumType, typename FlagType>
		requires std::is_enum_v    <EnumType> &&
	             std::is_unsigned_v<FlagType>
	inline bool IsFlagEnabled(EnumType a_checkFlag, FlagType a_flags)
	{
		const auto l_checkFlag = static_cast<FlagType>(a_checkFlag);

		return (a_flags & l_checkFlag) != static_cast<FlagType>(Constant::k_noFlagValue);
	}
}