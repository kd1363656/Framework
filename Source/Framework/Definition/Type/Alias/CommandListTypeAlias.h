#pragma once

namespace FWK::TypeAlias
{
	using DirectCommandList  = Graphics::CommandList<D3D12_COMMAND_LIST_TYPE_DIRECT>;
	using CopyCommandList    = Graphics::CommandList<D3D12_COMMAND_LIST_TYPE_COPY>;
	using ComputeCommandList = Graphics::CommandList<D3D12_COMMAND_LIST_TYPE_COMPUTE>;
}