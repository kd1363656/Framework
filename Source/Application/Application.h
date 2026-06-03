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

private:

	void LoadCONFIG    ();
	void PostLoadCONFIG();

	bool BeginFrame();

	void ProcessResizeRequest();

	bool CanUpdateFrame() const;

	void SaveCONFIG() const;

	const std::wstring k_windowClassName = L"Window";
	const std::string  k_titleName       = "MRI_FRAMEWORK";

	static constexpr int k_exitCodeSuccess			   =  0;
	static constexpr int k_exitCodeCOMInitializeFailed = -1;

	FWK::Window m_window = {};
};