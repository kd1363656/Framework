#include "Stopwatch.h"

FWK::Utility::Stopwatch::Stopwatch()
{
	m_startTime = std::chrono::steady_clock::now();
}

FWK::Utility::Stopwatch::~Stopwatch() = default;

double FWK::Utility::Stopwatch::FetchElapsedSecond() const
{
	const std::chrono::duration<double> l_elapsedTime = std::chrono::steady_clock::now() - m_startTime;

	return l_elapsedTime.count();
}