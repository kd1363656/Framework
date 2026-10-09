#pragma once

namespace Converter
{
    class ApplicationJsonConverter;
}

class Application final : public FWK::Utility::SingletonBase<Application>
{
private:

    friend class SingletonBase<Application>;

     Application();
    ~Application() override;

public:

    void Execute();

    const auto& GetREFAssetFilePathRegistry() const { return m_assetFilePathRegistry; }

    const auto& GetREFWindow       () const { return m_window; }
    const auto& GetREFFPSController() const { return m_fpsController; }

    auto& GetMutableREFAssetFilePathRegistry() { return m_assetFilePathRegistry; }

    static constexpr int k_exitCodeSuccess             =  0;
    static constexpr int k_exitCodeCOMInitializeFailed = -1;

private:

    void LoadCONFIG    ();
    void PostLoadCONFIG();

    bool BeginFrame();

    void EndFrame();

    void SaveCONFIG() const;

    void ClearWindowResizeRequest();

    void UpdateWindowTitleBar() const;

    bool CanUpdateFrame() const;

    inline static const std::string  k_titleName       = "MRI_FRAMEWORK";
    inline static const std::wstring k_windowClassName = L"Window";

    inline static const std::filesystem::path k_configFileIOPath       = "CONFIG/Application/ApplicationCONFIG.json";
    inline static const std::filesystem::path k_firstLoadSceneFilePath = "Asset/Data/Scene/Title/Title.json";
    inline static const std::filesystem::path k_firstLoadSceneName     = "Title";

    std::unique_ptr<Converter::ApplicationJsonConverter> m_jsonConverter;

    FWK::AssetFilePathRegistry m_assetFilePathRegistry;

    FWK::Window        m_window;
    FWK::FPSController m_fpsController;
};