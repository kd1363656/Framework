#pragma once

namespace FWK::Editor
{
	class EditorWindowPaneSplitter final
	{
	public:

		 EditorWindowPaneSplitter() = default;
		~EditorWindowPaneSplitter() = default;

		void Deserialize(const nlohmann::json& a_rootJson);
		
		nlohmann::json Serialize() const;

		void PreparePaneSize(const ImVec2&                                  a_availableContentRegion,
			                 const Enum::EditorWindowPaneSplitterResizeAxis a_resizeAxis           = Enum::EditorWindowPaneSplitterResizeAxis::X,
			                 const float                                    a_minPrimaryPaneSize   = k_defaultMinPrimaryPaneSize,
			                 const float                                    a_minSecondaryPaneSize = k_defaultMinSecondaryPaneSize);

		// PrimaryPaneとSecondaryPaneの間へSplitterを描画する
		// ResizeAxis::X : 左右Drag可能な縦Splitter
		// ResizeAxis::Y : 上下Drag可能な横Splitter
		// ResizeAxisを小ry買う↓場合はX軸として扱う
		void Draw(const std::string_view&                        a_label,
			      const ImVec2&                                  a_availableContentRegion,
			      const Enum::EditorWindowPaneSplitterResizeAxis a_resizeAxis           = Enum::EditorWindowPaneSplitterResizeAxis::X,
			      const float                                    a_minPrimaryPaneSize   = k_defaultMinPrimaryPaneSize,
			      const float                                    a_minSecondaryPaneSize = k_defaultMinSecondaryPaneSize);

		void SetPrimaryPaneSize(const float a_set) { m_primaryPaneSize = a_set; }

		const auto GetVALPrimaryPaneSize() const { return m_primaryPaneSize; }

	private:

		static constexpr float k_defaultMinPrimaryPaneSize   = 100.0F;
		static constexpr float k_defaultMinSecondaryPaneSize = 100.0F;

		static constexpr float k_splitterHitThickness = 6.0F;

		static constexpr float k_splitterLineThickness = 1.0F;

		static constexpr float k_centerDivisor = 2.0F;

		static constexpr float k_sameLineOffsetFromStartX = 0.0F;
		static constexpr float k_sameLineSpacing          = 0.0F;

		static constexpr float k_minDrawablePaneSize     = 1.0F;
		static constexpr float k_minDrawableSplitterSize = 1.0F;

		Converter::EditorWindowPaneSplitterJsonConverter m_jsonConverter = {};

		float m_primaryPaneSize = Constant::k_imguiDefaultPrimaryPaneSize;
	};
}