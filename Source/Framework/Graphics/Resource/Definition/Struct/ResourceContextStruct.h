#pragma once

namespace FWK::Struct
{
	struct AssetHeaderBase
	{
		std::uint64_t m_fileSize = Constant::k_emptyAssetFileSize;

		std::uint16_t m_version     = Constant::k_emptyAssetVersion;
		std::uint16_t m_assetTypeID = Constant::k_emptyAssetTypeID;
	};
}