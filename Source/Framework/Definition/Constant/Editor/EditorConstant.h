#pragma once

namespace FWK::Constant
{
	inline const std::filesystem::path k_contentRootFolderPath = "Content";

	inline constexpr std::string_view k_selectUnknownString = "Unknown";
	inline constexpr std::string_view k_isTrueString        = "True";
	inline constexpr std::string_view k_isFalseString       = "False";

	inline constexpr std::string_view k_gameObjectString = "GameObject";

	inline constexpr std::string_view k_gameObjectDragDropPayloadLabel       = "ゲームオブジェクト";
	inline constexpr std::string_view k_assetFilePathDragAndDropPayloadLabel = "AssetFilePath";
	
	inline constexpr std::string_view k_factoryCheckBoxListLabel = "##FactoryCheckBoxList";
	
	inline constexpr std::string_view k_fontAwesomeFolderIcon = "\xEF\x81\xBB";
	inline constexpr std::string_view k_fontAwesomeFileIcon   = "\xEF\x85\x9B";
	inline constexpr std::string_view k_fontAwesomeImageIcon  = "\xEF\x80\xBE";
	inline constexpr std::string_view k_fontAwesomeCubeIcon   = "\xEF\x86\xB2";

	inline constexpr ImVec2 k_defaultChildWindowSize = { 250.0F, 180.0F };

	inline constexpr ImVec2 k_nodeEditorTopCenterPinPivotAlignment    = { 0.50F, 0.0F };
	inline constexpr ImVec2 k_nodeEditorBottomCenterPinPivotAlignment = { 0.50F, 1.0F };
	inline constexpr ImVec2 k_nodeEditorLeftCenterPinPivotAlignment   = { 0.0F,  0.50F };
	inline constexpr ImVec2 k_nodeEditorRightCenterPinPivotAlignment  = { 1.0F,  0.50F };

	inline constexpr ImVec2 k_defaultNodeEditorCanvasSize = { 0.0F, 0.0F };

	inline constexpr TypeAlias::Math::Color k_debugINFOColor = 
	{
		0.92F,
		0.96F,
		0.92F,
		1.00F
	};

	inline constexpr TypeAlias::Math::Color k_debugSuccessColor =
	{
		0.35F,
		0.90F,
		0.45F,
		1.00F
	};

	inline constexpr TypeAlias::Math::Color k_debugWarningColor =
	{
		1.00F,
		0.78F,
		0.24F,
		1.00F
	};

	inline constexpr TypeAlias::Math::Color k_debugErrorColor =
	{
		1.00F,
		0.32F,
		0.32F,
		1.00F
	};

	inline constexpr float k_imguiDefaultDragValue       = 0.10F;
	inline constexpr float k_imguiDefaultDelayDrawSecond = 1.0F;

	// -1.0を使用することでウィンドウのX最大まで引き延ばす
	inline constexpr float k_childWindowMaxSizeX          = -1.0F;
	inline constexpr float k_defaultChildVisibleItemCount = 8.0F;

	inline constexpr std::size_t k_filePathNullTerminatorElementCount = 1ULL;
}