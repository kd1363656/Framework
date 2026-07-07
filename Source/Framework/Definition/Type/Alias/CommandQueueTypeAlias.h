#pragma once

namespace FWK::TypeAlias
{
	using DirectCommandQueue  = Graphics::CommandQueue<D3D12_COMMAND_LIST_TYPE_DIRECT>;
	using CopyCommandQueue    = Graphics::CommandQueue<D3D12_COMMAND_LIST_TYPE_COPY>;
	using ComputeCommandQueue = Graphics::CommandQueue<D3D12_COMMAND_LIST_TYPE_COMPUTE>;
}