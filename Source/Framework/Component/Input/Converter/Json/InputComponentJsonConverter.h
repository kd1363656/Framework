#pragma once

namespace FWK
{
	class InputComponent;
}

namespace FWK::Converter
{
	class InputComponentJsonConverter final
	{
	public:

		 InputComponentJsonConverter() = default;
		~InputComponentJsonConverter() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const;
		
		nlohmann::json SerializePrefab(const InputComponent& a_inputComponent) const;
	};
}