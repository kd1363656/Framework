#include "InputComponentInspector.h"

void FWK::InputComponentInspector::INIT()
{
	m_nodeEditor.INIT();
}

void FWK::InputComponentInspector::Deserialize(const nlohmann::json& a_rootJson)
{
	if (a_rootJson.is_null()) { return; }

	m_jsonConverter.Deserialize(a_rootJson, *this);
}

void FWK::InputComponentInspector::EditInspector(InputComponent& a_inputComponent)
{
	// まずはNodeGraph基盤を完成させるために
	
}

nlohmann::json FWK::InputComponentInspector::Serialize() const
{
	return m_jsonConverter.Serialize(*this);
}

bool FWK::InputComponentInspector::SynchronizeNodeGraph(InputComponent& a_inputComponent)
{
	return false;
}
bool FWK::InputComponentInspector::SynchronizeStartNode()
{
	return false;
}
bool FWK::InputComponentInspector::SynchronizeExecuteNode()
{
	return false;
}
bool FWK::InputComponentInspector::SynchronizeConditionNodeList(InputComponent& a_inputComponent)
{
	return false;
}
void FWK::InputComponentInspector::SynchronizeLinkList(const InputComponent& a_inputComponent)
{
}

void FWK::InputComponentInspector::ApplyDefaultNodePositions(const InputComponent& a_inputComponent)
{
}

void FWK::InputComponentInspector::DrawStartNode() const
{
}
void FWK::InputComponentInspector::DrawConditionNodeList(InputComponent& a_inputComponent) const
{
}
void FWK::InputComponentInspector::DrawExecuteNode() const
{
}

void FWK::InputComponentInspector::ProcessLinkCreation(const InputComponent& a_inputComponent)
{
}
void FWK::InputComponentInspector::ProcessLinkDestruction()
{
}

bool FWK::InputComponentInspector::FetchVALIsAllowedInputPin(const InputComponent& a_inputComponent, const TypeAlias::NodeEditorID a_inputPinID) const
{
	return false;
}