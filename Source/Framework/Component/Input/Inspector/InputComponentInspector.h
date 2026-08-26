#pragma once

namespace FWK
{
	class InputComponent;
}

namespace FWK
{
	class InputComponentInspector final
	{
	public:

		 InputComponentInspector() = default;
		~InputComponentInspector() = default;

		void INIT();

		void EditInspector(InputComponent& a_inputComponent);

	private:

		static constexpr std::string_view k_nodeEditorName = "##InputComponentNodeEditor";
		
		static constexpr float k_nodeEditorHeight = 300.0F;

		Editor::NodeEditor m_nodeEditor = {};
	};
}