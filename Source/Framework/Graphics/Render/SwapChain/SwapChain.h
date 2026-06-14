#pragma once

namespace FWK::Graphics
{
	class SwapChain final
	{
	public:

		 SwapChain() = default;
		~SwapChain() = default;

		void Deserialize(const nlohmann::json& a_rootJson);

		bool Create(const Window&						a_window,
					const Device&						a_device,
					const Factory&						a_factory,
					const DirectCommandQueue&			a_directCommandQueue,
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);

		void PostCreateSetup(const HWND& a_hwnd, const Factory& a_factory) const;

		void Present() const;

		nlohmann::json Serialize() const;

		// ※注意
		// 呼び出す前にGPUとの完全同期を取ること
		bool Resize(const Device&				        a_device, 
					const ResourceReleaseContext&       a_resourceReleaseContext,
					const Struct::ClientSize&	        a_clientSize, 
						  TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);

		void ResizeBackBufferList(const std::size_t a_backBufferNUM);

		void SetSyncInterval(const UINT a_set) { m_syncInterval = a_set; }

		UINT FetchVALCurrentBackBufferIndex() const;

		static constexpr auto GetVALDefaultBackBufferNUM() { return k_defaultBackBufferNUM; }
		static constexpr auto GetVALDefaultSyncInterval () { return k_defaultSyncInterval; }

		const auto& GetREFBackBufferList() const { return m_backBufferList; }

		const auto& GetREFSwapChain() const { return m_swapChain; }

		auto& GetMutableREFBackBufferList() { return m_backBufferList; }

		auto GetVALSyncInterval() const { return m_syncInterval; }

	private:

		bool CreateSwapChain(const Window& a_window, const Factory& a_factory, const DirectCommandQueue& a_directCommandQueue);

		bool CreateBackBufferList(const Device& a_device, TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool);

		bool IsValidBackBufferSize(const Struct::ClientSize& a_clientSize) const;

		static constexpr UINT k_defaultBackBufferNUM = 2U;
		static constexpr UINT k_invalidBackBufferNUM = std::numeric_limits<UINT>::max();

		static constexpr UINT k_defaultSyncInterval = 1U;

		static constexpr UINT k_swapChainPresentFlagNone = 0U;
		static constexpr UINT k_swapChainDescFlags	     = 0U;

		std::vector<Struct::BackBuffer> m_backBufferList = {};

		TypeAlias::ComPtr<IDXGISwapChain4> m_swapChain = nullptr;

		Converter::SwapChainJsonConverter m_jsonConverter = {};

		UINT m_syncInterval = k_defaultSyncInterval;
	};
}