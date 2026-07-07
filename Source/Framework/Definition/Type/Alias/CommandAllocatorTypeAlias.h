#pragma once

namespace FWK::TypeAlias
{
	using DirectCommandAllocator  = Graphics::CommandAllocator<D3D12_COMMAND_LIST_TYPE_DIRECT>;
	using CopyCommandAllocator    = Graphics::CommandAllocator<D3D12_COMMAND_LIST_TYPE_COPY>;
	using ComputeCommandAllocator = Graphics::CommandAllocator<D3D12_COMMAND_LIST_TYPE_COMPUTE>;
}