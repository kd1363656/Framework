#pragma once

namespace FWK::Struct
{
	struct AssetFilePathData final
	{
		std::filesystem::path m_assetFilePath = {};
		
		Enum::AssetFilePathRegistryType m_type = Enum::AssetFilePathRegistryType::Invalid;
	};
}