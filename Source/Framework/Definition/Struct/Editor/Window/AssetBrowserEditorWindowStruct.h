#pragma once

namespace FWK::Struct
{
	struct AssetBrowserEntryData final
	{
		std::filesystem::path m_entryPath = {};

		bool m_isFolder     = false;
		bool m_isSelectable = false;
	};
}