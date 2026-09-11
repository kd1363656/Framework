#pragma once

namespace FWK::Struct
{
	struct AssetBrowserFilePathChange final
	{
		std::filesystem::path m_oldFilePath = {};
		std::filesystem::path m_newFilePath = {};
	};
}