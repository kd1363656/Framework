#pragma once

namespace FWK::Utility
{
	class AssetFilePathHelper;
}

namespace FWK::Utility
{
	class AssetFilePathHelperInspector final
	{
	public:

		 AssetFilePathHelperInspector() = default;
		~AssetFilePathHelperInspector() = default;

		void EditInspector(AssetFilePathHelper& a_assetFilePathHelper);
	};
}