#pragma once

namespace FWK::Graphics
{
	template <D3D12_COMMAND_LIST_TYPE CommandType>
	class CommandAllocator
	{
	public:

		 CommandAllocator() = default;
		~CommandAllocator() = default;
	};
}