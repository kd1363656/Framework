#pragma once

namespace FWK::Constant
{
	inline constexpr std::string_view k_stringUnknown      = "Unknown";
	inline constexpr std::string_view k_imguiIsTrueString  = "True";
	inline constexpr std::string_view k_imguiIsFalseString = "False";

	inline constexpr std::string_view k_imguiGameObjectString = "GameObject";

	inline constexpr std::string_view k_imguiGameObjectDragDropPayloadLabel       = "ゲームオブジェクト";
	inline constexpr std::string_view k_imguiSceneDragDropPayloadLabel            = "シーン";
	inline constexpr std::string_view k_imguiAssetFilePathDragAndDropPayloadLabel = "AssetFilePath";
	
	inline constexpr std::string_view k_imguiFactoryCheckBoxListLabel = "##FactoryCheckBoxList";
	
	inline constexpr std::string_view k_imguiFontAwesomeFolderIcon = "\xEF\x81\xBB";
	inline constexpr std::string_view k_imguiFontAwesomeFileIcon   = "\xEF\x85\x9B";
	inline constexpr std::string_view k_imguiFontAwesomeImageIcon  = "\xEF\x80\xBE";
	inline constexpr std::string_view k_imguiFontAwesomeCubeIcon   = "\xEF\x86\xB2";

	inline constexpr ImVec2 k_imguiDefaultChildWindowSize = { 250.0F, 180.0F };

	inline constexpr ImVec2 k_imguiDefaultNodeEditorCanvasSize = { 0.0F, 0.0F };

	inline constexpr TypeAlias::Math::Color k_imguiDebugINFOColor = 
	{
		0.92F,
		0.96F,
		0.92F,
		1.00F
	};

	inline constexpr TypeAlias::Math::Color k_imguiDebugSuccessColor =
	{
		0.35F,
		0.90F,
		0.45F,
		1.00F
	};

	inline constexpr TypeAlias::Math::Color k_imguiDebugWarningColor =
	{
		1.00F,
		0.78F,
		0.24F,
		1.00F
	};

	inline constexpr TypeAlias::Math::Color k_imguiDebugErrorColor =
	{
		1.00F,
		0.32F,
		0.32F,
		1.00F
	};

	inline constexpr float k_imguiDefaultDragValue       = 0.10F;
	inline constexpr float k_imguiDefaultDelayDrawSecond = 1.0F;

	// -1.0を使用することでウィンドウのX最大まで引き延ばす
	inline constexpr float k_imguiChildWindowMAXSize           = -1.0F;
	inline constexpr float k_imguiDefaultChildVisibleItemCount = 8.0F;

	inline constexpr TypeAlias::NodeEditorID k_imguiInvalidNodeEditorID         = std::numeric_limits<TypeAlias::NodeEditorID>::max();
	inline constexpr TypeAlias::NodeEditorID k_imguiDefaultNodeEditorIDCapacity = 1000;
}