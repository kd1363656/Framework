#pragma once

namespace FWK::Struct
{
	struct ContentBrowserEntryData final
	{
		std::filesystem::path m_entryPath = {};

		bool m_isFolder     = false;
		bool m_isSelectable = false;
	};
}