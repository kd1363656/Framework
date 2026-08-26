#include "InputComponentJsonConverter.h"

void FWK::Converter::InputComponentJsonConverter::DeserializePrefab(const nlohmann::json& a_rootJson, InputComponent& a_inputComponent) const
{

}

nlohmann::json FWK::Converter::InputComponentJsonConverter::SerializePrefab(const InputComponent& a_inputComponent) const
{
	return nlohmann::json();
}