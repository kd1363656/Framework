#pragma once

namespace FWK::Graphics
{
	class Factory final
	{
	public:

		 Factory() = default;
		~Factory() = default;

		bool Create();

		const auto& GetREFFactory() const { return m_factory; }

	private:

		static constexpr UINT k_defaultFactoryCreateFlags = 0U;

		TypeAlias::ComPtr<IDXGIFactory7> m_factory = nullptr;
	};
}