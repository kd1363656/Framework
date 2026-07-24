#pragma once

namespace FWK::Graphics
{
	class SwapChain;
}

namespace FWK::Converter
{
	class SwapChainJsonConverter
	{
	public:

		struct BackBuffer final
		{
			TypeAlias::ComPtr<ID3D12Resource2> m_backBufferResource = nullptr;

			D3D12_RESOURCE_STATES m_currentResourceState = D3D12_RESOURCE_STATE_PRESENT;

			TypeAlias::DescriptorIndex m_rtvDescriptorIndex = Graphics::DescriptorHeap::k_invalidDescriptorIndex;
		};

	public:

		 SwapChainJsonConverter() = default;
		~SwapChainJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::SwapChain& a_swapChain) const;

		nlohmann::json Serialize(const Graphics::SwapChain& a_swapChain) const;

	private:

		static constexpr std::string_view k_backBufferNUMJsonKey = "BackBufferNum";
		static constexpr std::string_view k_syncIntervalJsonKey  = "SyncInterval";
	};
}