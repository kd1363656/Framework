#pragma once

namespace FWK::Editor
{
	class EditorStyle final
	{
	public:

		 EditorStyle() = default;
		~EditorStyle() = default;

		static void ApplySakuraDarkStyle();
		static void ApplyFont           ();

	private:

		static ImVec4 ConvertEditorColorToIMVEC4(const TypeAlias::Math::Color&a_color);

		static constexpr const char* k_japaneseFontPath = "C:/Windows/Fonts/meiryo.ttc";

		static constexpr float k_editorGlobalScale = 1.0F;

		static constexpr float k_windowRounding    = 4.0F;
		static constexpr float k_childRounding     = 4.0F;
		static constexpr float k_frameRounding     = 4.0F;
		static constexpr float k_popupRounding     = 4.0F;
		static constexpr float k_scrollbarRounding = 4.0F;
		static constexpr float k_grabRounding      = 4.0F;
		static constexpr float k_tabRounding       = 4.0F;

		static constexpr float k_windowBorderSize = 1.0F;
		static constexpr float k_childBorderSize  = 1.0F;
		static constexpr float k_popupBorderSize  = 1.0F;
		static constexpr float k_frameBorderSize  = 1.0F;
		static constexpr float k_tabBorderSize    = 1.0F;

		static constexpr float k_windowPaddingX = 8.0F;
		static constexpr float k_windowPaddingY = 8.0F;

		static constexpr float k_framePaddingX = 8.0F;
		static constexpr float k_framePaddingY = 5.0F;

		static constexpr float k_itemSpacingX = 8.0F;
		static constexpr float k_itemSpacingY = 6.0F;

		static constexpr float k_indentSpacing = 18.0F;
		static constexpr float k_scrollbarSize = 14.0F;

		static constexpr float k_editorFontSize = 16.0F;

		static constexpr TypeAlias::Math::Color k_clearColor =
		{
			0.00F,
			0.02F,
			0.01F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_backgroundColor =
		{
			0.01F,
			0.08F,
			0.03F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_panelColor =
		{
			0.02F,
			0.13F,
			0.05F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_panelHoverColor =
		{
			0.05F,
			0.24F,
			0.10F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_panelActiveColor =
		{
			0.08F,
			0.32F,
			0.14F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_borderColor =
		{
			0.15F,
			0.65F,
			0.25F,
			0.75F
		};

		static constexpr TypeAlias::Math::Color k_borderShadowColor =
		{
			0.00F,
			0.00F,
			0.00F,
			0.00F
		};

		static constexpr TypeAlias::Math::Color k_textColor =
		{
			0.92F,
			0.96F,
			0.92F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_textDisabledColor =
		{
			0.45F,
			0.58F,
			0.48F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_accentColor =
		{
			1.00F,
			0.25F,
			0.48F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_accentHoverColor =
		{
			1.00F,
			0.36F,
			0.58F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_accentActiveColor =
		{
			1.00F,
			0.18F,
			0.40F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_buttonColor =
		{
			0.03F,
			0.18F,
			0.07F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_buttonHoverColor =
		{
			0.08F,
			0.32F,
			0.14F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_buttonActiveColor =
		{
			0.10F,
			0.42F,
			0.18F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_headerColor =
		{
			0.04F,
			0.20F,
			0.08F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_headerHoverColor =
		{
			0.08F,
			0.32F,
			0.14F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_headerActiveColor =
		{
			0.12F,
			0.42F,
			0.20F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_tabColor =
		{
			0.02F,
			0.12F,
			0.05F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_tabHoverColor =
		{
			0.08F,
			0.28F,
			0.12F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_tabActiveColor =
		{
			0.05F,
			0.22F,
			0.09F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_frameColor =
		{
			0.02F,
			0.12F,
			0.05F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_frameHoverColor =
		{
			0.06F,
			0.28F,
			0.11F,
			1.00F
		};

		static constexpr TypeAlias::Math::Color k_frameActiveColor =
		{
			0.10F,
			0.38F,
			0.16F,
			1.00F
		};
	};
}