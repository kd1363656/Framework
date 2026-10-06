#pragma once

namespace FWK::Graphics
{
    class Device final
    {
    public:

         Device() = default;
        ~Device() = default;

        bool Create(const Factory& a_factory);

        const auto& GetREFAdapter() const { return m_adapter; }
        const auto& GetREFDevice () const { return m_device; }

    private:

#if defined(_DEBUG)
        static constexpr std::wstring_view k_deviceDebugLogSeparator      = L"\n===================================================================\n";
        static constexpr std::wstring_view k_selectedGPUNameDebugLogLabel = L"使用GPU : ";
#endif

        static constexpr UINT k_firstAdapterIndex = 0U;

        TypeAlias::ComPtr<IDXGIAdapter4> m_adapter = nullptr;
        TypeAlias::ComPtr<ID3D12Device9> m_device  = nullptr;
    };
}