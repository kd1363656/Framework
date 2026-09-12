#pragma once

class Application final : public FWK::Utility::SingletonBase<Application>
{
private:

    friend class SingletonBase<Application>;

     Application();
    ~Application() override;

public:

    void Execute();

    const auto& GetREFWindow       () const { return m_window; }
    const auto& GetREFFPSController() const { return m_fpsController; }

    static constexpr int k_exitCodeSuccess             =  0;
    static constexpr int k_exitCodeCOMInitializeFailed = -1;

private:

    void LoadCONFIG    ();
    void PostLoadCONFIG();

    bool BeginFrame();

    void EndFrame();

    void SaveCONFIG() const;

    void ClearWindowResizeRequest();

    bool CanUpdateFrame      () const;
    void UpdateWindowTitleBar() const;

    inline static const std::wstring k_windowClassName = L"Window";
    inline static const std::string  k_titleName       = "MRI_FRAMEWORK";

    static constexpr std::string_view k_firstLoadSceneFilepath = "Asset/Data/Scene/Game/Stage1-1.json";

    FWK::Window        m_window;
    FWK::FPSController m_fpsController;
};