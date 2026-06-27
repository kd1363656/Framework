#pragma once

namespace FWK::Struct
{
	struct StandardPipelineInputElement
	{
		D3D12_INPUT_ELEMENT_DESC m_inputElementDesc = {};

		std::string m_semanticName = {};
	};
}