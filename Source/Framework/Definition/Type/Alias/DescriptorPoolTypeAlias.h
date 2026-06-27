#pragma once

namespace FWK::TypeAlias
{
	using RTVDescriptorPool = Graphics::DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>;
	using SRVDescriptorPool = Graphics::DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>;
	using DSVDescriptorPool = Graphics::DescriptorPool<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>;
}