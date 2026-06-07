#pragma once

namespace FWK::Constant
{
	inline constexpr TypeAlias::StorageID k_invalidStorageID = std::numeric_limits<TypeAlias::StorageID>::max();

	inline constexpr TypeAlias::StorageID k_invalidStorageIDCapacity = 0U;

	inline constexpr UINT k_emptyAssetReferenceCount = 0U;

	inline constexpr std::uint32_t k_defaultAssetReferenceCount = 1U;
}