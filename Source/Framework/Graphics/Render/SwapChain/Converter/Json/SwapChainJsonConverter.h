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

		 SwapChainJsonConverter() = default;
		~SwapChainJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Graphics::SwapChain& a_swapChain) const;

		nlohmann::json Serialize(const Graphics::SwapChain& a_swapChain) const;

	private:

		static constexpr std::string_view k_backBufferNUMJsonKey = "BackBufferNum";
		static constexpr std::string_view k_syncIntervalJsonKey  = "SyncInterval";
	};
}