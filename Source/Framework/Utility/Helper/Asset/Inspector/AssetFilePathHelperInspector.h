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

		void EditInspector(AssetFilePathHelper& a_assetFilePathHelper) const;

	private:

		static constexpr std::string_view k_assetFilePathDragDropAreaID    = "##AssetFilePathDragDrop";
		static constexpr std::string_view k_assetFilePathDropAreaEmptyText = "コンテンツブラウザーからアセットファイルをドロップ";

		static constexpr float k_assetFilePathDropAreaHeight   = 48.0F;
		static constexpr float k_assetFilePathDropAreaMINWidth = 1.0F;
	};
}