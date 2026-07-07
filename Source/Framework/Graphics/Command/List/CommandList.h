#pragma once

namespace FWK::Graphics
{
	template <D3D12_COMMAND_LIST_TYPE CommandType>
	class CommandList
	{
	public:

		 CommandList() = default;
		~CommandList() = default;
	};
}