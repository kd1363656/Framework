#pragma once

class Application final : public FWK::Utility::SingletonBase<Application>
{
private:

	friend class SingletonBase<Application>;

	 Application()          = default;
	~Application() override = default;

public:

	void Execute();

	static constexpr int GetVALExitCodeSuccess			  () { return k_exitCodeSuccess; }
	static constexpr int GetVALExitCodeCOMInitializeFailed() { return k_exitCodeCOMInitializeFailed; }

	const auto& GetREFFFPSController() const { return m_fpsController; }

private:

	void LoadCONFIG    ();
	void PostLoadCONFIG();

	bool BeginFrame();
	void EndFrame  ();

	void SaveCONFIG() const;

	void ClearWindowResizeRequest();

	bool CanUpdateFrame() const;

	void UpdateWindowTitleBar() const;

	static constexpr std::string_view k_firstLoadSceneFilepath = "Asset/Data/Scene/Test/TestScene.json";

	static constexpr int k_exitCodeSuccess			   =  0;
	static constexpr int k_exitCodeCOMInitializeFailed = -1;

	const std::wstring k_windowClassName = L"Window";
	const std::string  k_titleName       = "MRI_FRAMEWORK";

	FWK::Window		   m_window		   = {};
	FWK::FPSController m_fpsController = {};
};