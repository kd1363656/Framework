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

		// ID3D12Deviceを使って作るコマンドキューなどの各種GPUオブジェクトで
		// 共通使用する既定のGPUノード指定値
		// GPUノードマスクは「どのGPUノードを対象にするか」を表す値、
		// このプロジェクトでは単一GPU前提で作成するため固定値にしている
		static constexpr DXGI_FORMAT k_defaultSwapChainBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		static constexpr UINT k_defaultSampleCount   = 1U;
		static constexpr UINT k_defaultSampleQuality = 0U;

	private:

		static constexpr std::string_view k_backBufferNUMJsonKey = "BackBufferNum";
		static constexpr std::string_view k_syncIntervalJsonKey  = "SyncInterval";
	};
}