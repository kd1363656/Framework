#include "FPSController.h"

void FWK::FPSController::LoadCONFIG()
{
	const auto& l_rootJson = Utility::LoadJsonFile(k_configFileIOPath);

	if (l_rootJson.is_null()) { return; }

	m_fpsControllerJsonConverter.Deserialize(l_rootJson, *this);
}

void FWK::FPSController::BeginFrame()
{
	// 現在のフレームでの時間
	m_frameBeginTime = std::chrono::steady_clock::now();

	// 1フレーム前の時間と現フレームでの時間の差分を求める(デルタタイム)
	const std::chrono::duration<float> l_elapsed = m_frameBeginTime - m_previousTime;

	// 秒単位で取得(ミリ秒で取得する必要がある)
	m_deltaTime = l_elapsed.count();

	// TimeScaleの影響を受けるデルタタイムを計算
	m_scaledDeltaTime = m_deltaTime * m_timeScale;
}
void FWK::FPSController::EndFrame()
{
	// フレームレートを一定に保つためにスリープ
	LimitFramerate();

	// もしデルタタイムが0.0以下の値を取る場合それは前回の時間と今回計測した時間の差分が
	// ないためm_targetFPSに現在のFPSが達している、0.0よりもデルタタイム大きければ
	// 計算を行い現在のFPSを算出する
	m_currentFPS = m_deltaTime > k_minDeltaTime ? k_second / m_deltaTime : m_targetFPS;

	// 過去の経過時間として現在の時刻を入れる
	m_previousTime = m_frameBeginTime;
}

void FWK::FPSController::SaveCONFIG() const
{
	const auto& l_rootJson = m_fpsControllerJsonConverter.Serialize(*this);

	Utility::SaveJsonFile(l_rootJson, k_configFileIOPath);
}

void FWK::FPSController::LimitFramerate() const
{
	// 現在の時刻を取得
	const auto& l_currentTime = std::chrono::steady_clock::now();

	// 1000ミリ秒 / 目標FPSで今回使用すべき時間を算出
	const auto& l_frameTime = std::chrono::milliseconds{ static_cast<int>(k_milliSecond / m_targetFPS) };

	// 現在の時間を過去の時間と引いてやることで経過時間を算出
	const auto& l_elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(l_currentTime - m_frameBeginTime);

	if (l_elapsedTime >= l_frameTime) { return; }

	// 使用しきれていない、今回の処理が速く想定のFPSを超えてしまわないようにSleep
	std::this_thread::sleep_for(l_frameTime - l_elapsedTime);
}