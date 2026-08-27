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

		void Deserialize(const nlohmann::json& a_rootJson);

		void EditInspector(InputComponent& a_inputComponent);

		nlohmann::json Serialize() const;

	private:

		Editor::NodeEditor m_nodeEditor = {};
		
		Converter::InputComponentInspectorJsonConverter m_jsonConverter = {};
	};
}