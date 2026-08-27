#pragma once

namespace FWK
{
	class InputComponentInspector;
}

namespace FWK::Converter
{
	class InputComponentInspectorJsonConverter final
	{
	public:

		 InputComponentInspectorJsonConverter() = default;
		~InputComponentInspectorJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, InputComponentInspector& a_inputComponentInspector) const;
		
		nlohmann::json Serialize(const InputComponentInspector& a_inputComponentInspector) const;

	private:

		static constexpr std::string_view k_nodeEditorJsonKey = "InputComponentNodeEditor";
	};	
}